#include <stddef.h>

#include "comm.h"
#include "atomic.h"
#include "mem.h"

#include "comm_private.h"

int comm_init(comm_t *cc, uint64_t length, int type) {
    cc->total_length = length;
    cc->flags = 0;
    cc->ring_begin = cc->ring_end = 0;
    if(type == COMM_SIMPLE) {
        cc->flags = type;
        cc->data_begin = offsetof(comm_t, simple.last);
        cc->data_begin = (cc->data_begin + 127) & ~0x7f;
    }
    else if(type == COMM_MULTI) {
        cc->flags = type;
        // NYI
        while(1) {}
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
