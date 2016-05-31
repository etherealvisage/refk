#ifndef COMM_PRIVATE_H
#define COMM_PRIVATE_H

#include <stdint.h>

#define COMM_TYPE_MASK          0x03
#define COMM_TYPE_SHIFT         0
#define COMM_BUCKETSIZE_MASK    0xff00
#define COMM_BUCKETSIZE_SHIFT   8

struct comm_t {
    // constants
    uint64_t total_length;
    uint64_t data_begin, data_length;
    uint64_t flags;

    // multiple-thread values
    union {
        volatile uint64_t ring_begin;
        volatile uint64_t buckets_filled;
    } __attribute__((aligned(128)));
    union {
        volatile uint64_t ring_end;
        volatile uint64_t buckets_processed;
    } __attribute__((aligned(128)));

    union {
        struct {
            char last[0];
        } simple;
        struct {
            // semaphore
            uint64_t packet_count;

            char last[0];
        } multi;
        struct {
            uint64_t buckets;
            uint64_t bucket_size;
            // TODO: move these out of the comm_t structure
            // TODO: these need to be in two separate cache lines, not four
            int64_t __attribute__((aligned(128))) current_read;
            int64_t __attribute__((aligned(128))) current_read_offset;
            int64_t __attribute__((aligned(128))) current_write;
            int64_t __attribute__((aligned(128))) current_write_offset;
            char last[0];
        } bucketed;
    };
};

int comm_put(struct comm_t *cc, void *data, uint64_t data_size);
int comm_peek(struct comm_t *cc, void *data, uint64_t *data_size);

int comm_bucketread(struct comm_t *cc, void *data, uint64_t *data_size);
int comm_bucketwrite(struct comm_t *cc, void *data, uint64_t data_size);
void comm_bucketflush(struct comm_t *cc);

#endif
