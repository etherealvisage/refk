#include "kcomm.h"
#include "kutil.h"

struct kcomm_t {
    uint64_t total_length;
    uint64_t data_begin, data_length;

    uint64_t ring_begin, ring_end;
};

void kcomm_init(kcomm_t *kc, uint64_t length) {
    kc->total_length = length;
    kc->data_begin = sizeof(kcomm_t);
    kc->data_length = length - kc->data_begin;

    kc->ring_begin = kc->ring_end = 0;
}

static uint64_t kcomm_remaining(kcomm_t *kc) {
    if(kc->ring_begin <= kc->ring_end) {
        uint64_t used = (kc->ring_end - kc->ring_begin);
        return kc->data_length - used;
    }
    else {
        return kc->ring_begin - kc->ring_end;
    }
}

static void kcomm_put_data(kcomm_t *kc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(kc->ring_end + data_size >= kc->data_length) {
        uint64_t before_wrap = kc->data_length - kc->ring_end;
        memcpy((uint8_t *)(kc) + kc->data_begin + kc->ring_end, data, before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        kc->ring_end = 0;
    }

    memcpy((uint8_t *)(kc) + kc->data_begin + kc->ring_end, data, data_size);
    kc->ring_end += data_size;
}

static void kcomm_get_data(kcomm_t *kc, void *data, uint64_t data_size) {
    // are we wrapping?
    if(kc->ring_begin + data_size >= kc->data_length) {
        uint64_t before_wrap = kc->data_length - kc->ring_begin;
        memcpy(data, (uint8_t *)(kc) + kc->data_begin + kc->ring_begin, before_wrap);
        data = (uint8_t *)data + before_wrap;
        data_size -= before_wrap;
        kc->ring_begin = 0;
    }

    memcpy(data, (uint8_t *)(kc) + kc->data_begin + kc->ring_begin, data_size);
    kc->ring_begin += data_size;
}

int kcomm_put(kcomm_t *kc, void *data, uint64_t data_size) {
    // is there enough space left?
    uint64_t req_size = data_size + 4;
    if(kcomm_remaining(kc) < req_size) return 1;

    uint32_t dsize = data_size;
    kcomm_put_data(kc, &dsize, sizeof(dsize));
    kcomm_put_data(kc, data, data_size);

    return 0;
}

int kcomm_get(kcomm_t *kc, void *data, uint64_t *data_size) {
    // is there any data waiting?
    if(kc->ring_begin == kc->ring_end) return 1;
    
    uint32_t dsize;
    kcomm_get_data(kc, &dsize, sizeof(dsize));
    kcomm_get_data(kc, data, dsize);

    *data_size = dsize;

    return 0;
}
