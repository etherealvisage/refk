#ifndef SCHEDULER_INTERFACE_H
#define SCHEDULER_INTERFACE_H

#include <stdint.h>

enum {
    SCHED_SET_NAME,
    SCHED_GET_NAMED,
    SCHED_SPAWN,
    SCHED_SET_STATE,
    SCHED_REAP,
};

enum {
    SCHED_STATE_RAX,
    SCHED_STATE_RBX,
    SCHED_STATE_RCX,
    SCHED_STATE_RDX,
    SCHED_STATE_RSI,
    SCHED_STATE_RDI,
    SCHED_STATE_RSP,
    SCHED_STATE_RBP,
    SCHED_STATE_R8,
    SCHED_STATE_R9,
    SCHED_STATE_R10,
    SCHED_STATE_R11,
    SCHED_STATE_R12,
    SCHED_STATE_R13,
    SCHED_STATE_R14,
    SCHED_STATE_R15,
    SCHED_STATE_RFLAGS,
    SCHED_STATE_RIP,
    SCHED_STATE_CS,
    SCHED_STATE_DS,
    SCHED_STATE_ES,
    SCHED_STATE_FS,
    SCHED_STATE_GS,
    SCHED_STATE_SS,
    SCHED_STATE_FS_BASE,
    SCHED_STATE_GS_BASE,
    SCHED_STATE_CR3,
    SCHED_STATE
};

typedef struct sched_in_packet_t {
    uint8_t type;
    uint64_t req_id;
    union {
        struct {
            uint64_t task_id;
            char name[32];
        } set_name;
        struct {
            char name[32];
        } get_named;
        struct {
            uint64_t root_id;
        } spawn;
        struct {
            uint64_t task_id;
            uint64_t index;
            uint64_t value;
        } set_state;
        struct {
            uint64_t task_id;
        } reap;
    };
} sched_in_packet_t;

typedef struct sched_out_packet_t {
    uint8_t type;
    uint64_t req_id;
    union {
        struct {
            uint64_t task_id;
        } get_named;
        struct {
            uint64_t task_id;
            uint64_t root_id;
        } spawn;
    };
} sched_out_packet_t;

#endif
