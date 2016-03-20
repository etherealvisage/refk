#include "comm.h"

#include "clib/comm_private.h"

int comm_read(comm_t *cc, void *data, uint64_t *data_size) {
    if((cc->flags & COMM_TYPE_MASK) == COMM_SIMPLE) {
        // simple case?
        return comm_peek(cc, data, data_size);
    }

    return 1;
}

int comm_write(comm_t *cc, void *data, uint64_t data_size) {
    if((cc->flags & COMM_TYPE_MASK) == COMM_SIMPLE) {
        // simple case?
        return comm_put(cc, data, data_size);
    }

    return 1;
}
