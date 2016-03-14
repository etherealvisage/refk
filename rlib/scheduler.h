#ifndef RLIB_SCHEDULER_H
#define RLIB_SCHEDULER_H

#include "mman.h"

typedef struct rlib_task_t {
    uint64_t task_id;
    uint64_t root_id;
} rlib_task_t;

#define RLIB_NEW_MEMSPACE (void *)0

void rlib_create_task(rlib_memory_space_t *memspace, rlib_task_t *task);
void rlib_set_local_task(rlib_task_t *task, void (*function)(void *),
    void *data, uint64_t stack_size);
void rlib_ready_task(rlib_task_t *task);

void rlib_reap_self();

void rlib_wait(uint64_t *pointer, uint64_t value);
void rlib_wake(uint64_t *pointer, uint64_t value, uint64_t count);

void rlib_process_queued();
void rlib_yield();

#endif
