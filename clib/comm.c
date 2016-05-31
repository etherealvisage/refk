#include <stddef.h>

#include "comm.h"
#include "atomic.h"
#include "mem.h"

#include "comm_private.h"

int comm_init(comm_t *cc, uint64_t length, int flags) {
    cc->total_length = length;
    cc->flags = flags;
    cc->ring_begin = cc->ring_end = 0;
    if((flags & COMM_TYPE_MASK) == COMM_SIMPLE) {
        cc->data_begin = offsetof(comm_t, simple.last);
        cc->data_begin = (cc->data_begin + 127) & ~0x7f;
    }
    else if((flags & COMM_TYPE_MASK) == COMM_MULTI) {
        // NYI
        while(1) {}
    }
    else if((flags & COMM_TYPE_MASK) == COMM_BUCKETED) {
        cc->data_begin = offsetof(comm_t, bucketed.last);
        cc->data_begin = (cc->data_begin + 127) & ~0x7f;

        uint64_t bucket_size = 128;
        bucket_size <<= (flags&COMM_BUCKETSIZE_MASK) >> COMM_BUCKETSIZE_SHIFT;

        cc->bucketed.bucket_size = bucket_size;
        cc->bucketed.buckets = (length - cc->data_begin) / bucket_size;

        cc->bucketed.current_read = -1;
        cc->bucketed.current_write = -1;
    }
    else {
        // invalid
        return 1;
    }

    cc->data_length = length - cc->data_begin;

    return 0;
}

static uint64_t comm_remaining(comm_t *cc) {
    if(cc->ring_begin == cc->ring_end) return cc->data_length;
    else if(cc->ring_begin < cc->ring_end) {
        uint64_t used = (cc->ring_end - cc->ring_begin);
        return cc->data_length - used;
    }
    else {
        return cc->ring_begin - cc->ring_end;
    }
}

static void comm_put_data(comm_t *cc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(cc->ring_end + data_size >= cc->data_length) {
        uint64_t before_wrap = cc->data_length - cc->ring_end;
        mem_copy((uint8_t *)(cc) + cc->data_begin + cc->ring_end, data,
            before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        cc->ring_end = 0;
    }

    mem_copy((uint8_t *)(cc) + cc->data_begin + cc->ring_end, data, data_size);
    cc->ring_end += data_size;
}

static void comm_get_data(comm_t *cc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(cc->ring_begin + data_size >= cc->data_length) {
        uint64_t before_wrap = cc->data_length - cc->ring_begin;
        mem_copy(data, (uint8_t *)(cc) + cc->data_begin + cc->ring_begin, before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        cc->ring_begin = 0;
    }

    mem_copy(data, (uint8_t *)(cc) + cc->data_begin + cc->ring_begin, data_size);
    cc->ring_begin += data_size;
}

static void comm_skip_data(comm_t *cc, uint64_t data_size) {
    // are we wrapping?
    if(cc->ring_begin + data_size >= cc->data_length) {
        cc->ring_begin = 0;
    }

    cc->ring_begin += data_size;
}

int comm_put(comm_t *cc, void *data, uint64_t data_size) {
    // is there enough space left?
    uint64_t req_size = data_size + 4;
    if(comm_remaining(cc) < req_size) return 1;

    uint32_t dsize = data_size;
    comm_put_data(cc, &dsize, sizeof(dsize));
    comm_put_data(cc, data, data_size);

    //atomic_inc(&cc->simple.packet_count);

    return 0;
}

int comm_peek(comm_t *cc, void *data, uint64_t *data_size) {
    // is there any data waiting?
    //if(cc->simple.packet_count == 0) return 1;
    //atomic_dec(&cc->simple.packet_count);
    if(cc->ring_begin == cc->ring_end) return 1;

    uint32_t dsize;
    comm_get_data(cc, &dsize, sizeof(dsize));
    if(*data_size < dsize) {
        comm_skip_data(cc, dsize);
        *data_size = dsize;
        return 1;
    }
    else {
        comm_get_data(cc, data, dsize);
        *data_size = dsize;
        return 0;
    }
}

static void *comm_bucket(struct comm_t *cc, uint64_t index) {
    uint64_t off = cc->data_begin + index * cc->bucketed.bucket_size;

    return (void *)((char *)cc + off);
}

static int comm_select_read_bucket(struct comm_t *cc) {
    uint64_t filled = cc->buckets_filled;
    uint64_t processed = cc->buckets_processed;
    if(filled > processed) {
        cc->bucketed.current_read = (processed + 1) % cc->bucketed.buckets;
        cc->bucketed.current_read_offset = 0;
        return 0;
    }
    else {
        cc->bucketed.current_read = -1;
        return 1;
    }
}

static int comm_select_write_bucket(struct comm_t *cc) {
    uint64_t filled = cc->buckets_filled;
    uint64_t processed = cc->buckets_processed;
    if(filled - processed < cc->bucketed.buckets) {
        cc->bucketed.current_write = (filled + 1) % cc->bucketed.buckets;
        cc->bucketed.current_write_offset = 0;
        return 0;
    }
    else {
        cc->bucketed.current_write = -1;
        return 1;
    }
}

int comm_bucketread(struct comm_t *cc, void *data, uint64_t *data_size) {
    // do we need a new bucket to read from?
    if(cc->bucketed.current_read == -1) {
        if(comm_select_read_bucket(cc)) return 1;
    }

    void *bucket = comm_bucket(cc, cc->bucketed.current_read);
    int64_t total = *(uint64_t *)bucket;

    // is this bucket finished?
    if(cc->bucketed.current_read_offset >= total) {
        cc->bucketed.current_read = -1;
        cc->buckets_processed ++;
        // tail-call recurse
        return comm_bucketread(cc, data, data_size);
    }

    uint64_t off = cc->bucketed.current_read_offset;

    // read data size
    uint64_t read_size = *(uint64_t *)((char *)bucket + off);
    // is there enough space?
    if(read_size > *data_size) {
        off += 8;
        off += read_size;
        cc->bucketed.current_read_offset = off;
        return 1;
    }

    off += 8;
    mem_copy(data, (char *)bucket + off, read_size);
    off += read_size;
    cc->bucketed.current_read_offset = off;

    return 0;
}

int comm_bucketwrite(struct comm_t *cc, void *data, uint64_t data_size) {
    // do we need a new bucket to write into?
    if(cc->bucketed.current_write == -1) {
        if(comm_select_write_bucket(cc)) return 1;
    }

    void *bucket = comm_bucket(cc, cc->bucketed.current_write);
    uint64_t *total = (uint64_t *)bucket;

    // is this bucket finished?
    if(cc->bucketed.current_write_offset + data_size 
        >= cc->bucketed.bucket_size) {

        *total = cc->bucketed.current_write_offset;

        cc->bucketed.current_write = -1;
        cc->buckets_filled ++;
        // tail-call recurse
        return comm_bucketwrite(cc, data, data_size);
    }

    uint64_t off = cc->bucketed.current_write_offset;

    // write data size
    *(uint64_t *)((char *)bucket + off) = data_size;

    mem_copy(data, (char *)bucket + off + 8, data_size);
    cc->bucketed.current_write_offset = off + 8 + data_size;

    return 0;
}

void comm_bucketflush(struct comm_t *cc) {
    // default value: 8 (total size)
    if(cc->bucketed.current_write_offset != 8) {
        void *bucket = comm_bucket(cc, cc->bucketed.current_write);
        uint64_t *total = (uint64_t *)bucket;

        *total = cc->bucketed.current_write_offset;

        cc->bucketed.current_write = -1;
        cc->buckets_filled ++;
    }
}
