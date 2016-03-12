#include "scheduler/interface.h"

#include "scheduler.h"
#include "kcomm.h"
#include "sequence.h"

#include "mman_private.h"

struct rlib_task_t {
    uint64_t task_id;
    uint64_t root_id;
};

void rlib_create_task(rlib_memory_space_t *memspace, rlib_task_t *task) {
    if(!task) return;

    sched_in_packet_t in;
    in.type = SCHED_SPAWN;
    in.req_id = rlib_sequence();
    
    if(memspace == RLIB_NEW_MEMSPACE) {
        in.spawn.root_id = 0;
    }
    else {
        in.spawn.root_id = memspace->root_id;
    }

    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    kcomm_put(schedin, &in, sizeof(in));

    sched_out_packet_t out;
    uint64_t length = sizeof(out);
    while(kcomm_get(schedout, &out, &length) || out.req_id != in.req_id) {
        length = sizeof(out);
    }

    task->task_id = out.spawn.task_id;
    task->root_id= out.spawn.task_id;
}
