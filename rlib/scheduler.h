#ifndef RLIB_SCHEDULER_H
#define RLIB_SCHEDULER_H

#include "mman.h"

typedef struct rlib_task_t rlib_task_t;

#define RLIB_NEW_MEMSPACE (void *)0

void rlib_create_task(rlib_memory_space_t *memspace, rlib_task_t *task);

#endif
