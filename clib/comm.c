#include "comm.h"
#include "atomic.h"
#include "mem.h"

struct comm_t {
    uint64_t total_length;
    uint64_t data_begin, data_length;

    uint64_t write_access, read_access;
    uint64_t packet_count;

    uint64_t ring_begin, ring_end;
};

void comm_init(comm_t *kc, uint64_t length) {
    kc->total_length = length;
    kc->data_begin = sizeof(comm_t);
    kc->data_length = length - kc->data_begin;

    kc->read_access = 1;
    kc->write_access = 1;
    kc->packet_count = 0;

    kc->ring_begin = kc->ring_end = 0;
}

static uint64_t comm_remaining(comm_t *kc) {
    if(kc->ring_begin <= kc->ring_end) {
        uint64_t used = (kc->ring_end - kc->ring_begin);
        return kc->data_length - used;
    }
    else {
        return kc->ring_begin - kc->ring_end;
    }
}

static void comm_put_data(comm_t *kc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(kc->ring_end + data_size >= kc->data_length) {
        uint64_t before_wrap = kc->data_length - kc->ring_end;
        mem_copy((uint8_t *)(kc) + kc->data_begin + kc->ring_end, data,
            before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        kc->ring_end = 0;
    }

    mem_copy((uint8_t *)(kc) + kc->data_begin + kc->ring_end, data, data_size);
    kc->ring_end += data_size;
}

static void comm_get_data(comm_t *kc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(kc->ring_begin + data_size >= kc->data_length) {
        uint64_t before_wrap = kc->data_length - kc->ring_begin;
        mem_copy(data, (uint8_t *)(kc) + kc->data_begin + kc->ring_begin, before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        kc->ring_begin = 0;
    }

    mem_copy(data, (uint8_t *)(kc) + kc->data_begin + kc->ring_begin, data_size);
    kc->ring_begin += data_size;
}

static void comm_skip_data(comm_t *kc, uint64_t data_size) {
    // are we wrapping?
    if(kc->ring_begin + data_size >= kc->data_length) {
        kc->ring_begin = 0;
    }

    kc->ring_begin += data_size;
}

int comm_put(comm_t *kc, void *data, uint64_t data_size) {
    // is there enough space left?
    uint64_t req_size = data_size + 4;
    if(comm_remaining(kc) < req_size) return 1;

    uint32_t dsize = data_size;
    comm_put_data(kc, &dsize, sizeof(dsize));
    comm_put_data(kc, data, data_size);

    return 0;
}

int comm_get(comm_t *kc, void *data, uint64_t *data_size) {
    // is there any data waiting?
    if(kc->ring_begin == kc->ring_end) return 1;
    
    uint32_t dsize;
    comm_get_data(kc, &dsize, sizeof(dsize));
    if(*data_size < dsize) {
        comm_skip_data(kc, dsize);
        *data_size = dsize;
    }
    else {
        comm_get_data(kc, data, dsize);
    }
    *data_size = dsize;

    return 0;
}
