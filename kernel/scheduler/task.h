#ifndef SCHEDULER_TASK_H
#define SCHEDULER_TASK_H

#include <stdint.h>

#include "klib/kcomm.h"

void task_init();

uint64_t sched_task_attach(task_state_t *ts, kcomm_t **sin, kcomm_t **sout);

void sched_set_name(uint64_t task_id, const char *name);
uint64_t sched_named_task(const char *name);
uint64_t sched_task_create(uint64_t root_id, kcomm_t **sin, kcomm_t **sout);
void sched_task_reap(uint64_t task_id);

void sched_set_state(uint64_t task_id, uint64_t index, uint64_t value);

#endif
