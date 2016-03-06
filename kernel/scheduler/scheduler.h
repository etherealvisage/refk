#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define COMM_BASE_ADDRESS 0xC044ADD4000
#define COMM_OUT_OFFSET 0x800

#include "klib/task.h"

enum {
    COMM_FORWARD,
    COMM_SPAWN,
};

typedef struct comm_in_packet_t {
    uint8_t type;
    union {
        struct {
            uint64_t to;
            uint64_t length;
            char message[32];
        } forward;
        struct {
            
        } spawn;
    };
} comm_in_packet_t;

typedef struct comm_out_packet_t {
    uint8_t type;
    union {
        struct {
            task_state_t *task;
        } spawn;
    };
} comm_out_packet_t;

#endif
