#ifndef COMM_PRIVATE_H
#define COMM_PRIVATE_H

#include <stdint.h>

#define COMM_TYPE_MASK      0x01

struct comm_t {
    // constants
    uint64_t total_length;
    uint64_t data_begin, data_length;
    uint64_t flags;

    uint64_t ring_begin;
    uint64_t ring_end;

    union {
        struct {
            // atomic counter
            uint64_t packet_count;

            char last[0];
        } simple;
        struct {
            // semaphore
            uint64_t packet_count;
        } multi;
    };
};

int comm_put(struct comm_t *cc, void *data, uint64_t data_size);
int comm_peek(struct comm_t *cc, void *data, uint64_t *data_size);

#endif
