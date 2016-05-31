#ifndef COMM_PRIVATE_H
#define COMM_PRIVATE_H

#include <stdint.h>

#define COMM_TYPE_MASK      0x01

struct comm_t {
    // constants
    uint64_t total_length;
    uint64_t data_begin, data_length;
    uint64_t flags;

    // multiple-thread values
    uint64_t __attribute__((aligned(128))) ring_begin;
    uint64_t __attribute__((aligned(128))) ring_end;

    union {
        struct {
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
