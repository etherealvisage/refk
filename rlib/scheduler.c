#include "klib/task.h"
#include "klib/d.h" // debugging

#include "kernel/scheduler/interface.h"

#include "comm.h"

#include "scheduler.h"
#include "sequence.h"

#include "mman_private.h"

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
    comm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);
    rlib_process_queued();

    sched_out_packet_t out;
    uint64_t length = sizeof(out);
    while(comm_read(schedout, &out, &length, 0) || out.req_id != in.req_id) {
        length = sizeof(out);
    }

    task->task_id = out.spawn.task_id;
    task->root_id = out.spawn.root_id;
}

static void rlib_local_task_wrapper(void (*function)(void *), void *context) {
    function(context);

    rlib_reap_self();
}

void rlib_set_local_task(rlib_task_t *task, void (*function)(void *),
    void *data, uint64_t stack_size) {

    comm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    sched_in_packet_t in;
    in.type = SCHED_SET_STATE;
    in.req_id = rlib_sequence();
    in.set_state.task_id = task->task_id;

    // set initial parameters
    in.set_state.index = SCHED_STATE_RDI;
    in.set_state.value = (uint64_t)function;
    comm_write(schedin, &in, sizeof(in));
    in.req_id = rlib_sequence();
    in.set_state.index = SCHED_STATE_RSI;
    in.set_state.value = (uint64_t)data;
    comm_write(schedin, &in, sizeof(in));
    in.req_id = rlib_sequence();
    in.set_state.index = SCHED_STATE_RIP;
    in.set_state.value = (uint64_t)rlib_local_task_wrapper;
    comm_write(schedin, &in, sizeof(in));
    in.req_id = rlib_sequence();
    in.set_state.index = SCHED_STATE_RSP;
    stack_size = (stack_size + 0xfff) & ~0xfff;
    in.set_state.value = rlib_anonymous(0, stack_size) + stack_size;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();

    sched_out_packet_t out;
    uint64_t length = sizeof(out);
    while(comm_read(schedout, &out, &length, 0) || out.req_id != in.req_id) {
        length = sizeof(out);
    }
}

void rlib_ready_task(rlib_task_t *task) {
    comm_t *schedin;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));

    sched_in_packet_t in;
    in.type = SCHED_SET_STATE;
    in.req_id = rlib_sequence();
    in.set_state.task_id = task->task_id;
    in.set_state.index = SCHED_STATE;
    in.set_state.value = TASK_STATE_VALID | TASK_STATE_RUNNABLE;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();
}

void rlib_ready_ap_task(rlib_task_t *task) {
    comm_t *schedin;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));

    sched_in_packet_t in;
    in.type = SCHED_SET_STATE;
    in.req_id = rlib_sequence();
    in.set_state.task_id = task->task_id;
    in.set_state.index = SCHED_STATE;
    in.set_state.value =
        TASK_STATE_VALID | TASK_STATE_RUNNABLE | TASK_STATE_APTASK;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();
}

void rlib_reap_self() {
    comm_t *schedin;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));

    sched_in_packet_t in;
    in.type = SCHED_REAP;
    in.req_id = rlib_sequence();
    in.reap.task_id = 0;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();

    // should never be reached!
    while(1) {}
}

void rlib_wait(uint64_t *pointer, uint64_t value) {
    comm_t *schedin;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));

    sched_in_packet_t in;
    in.type = SCHED_WAIT;
    in.req_id = rlib_sequence();
    in.wait.address = (uint64_t)pointer;
    in.wait.value = (uint64_t)value;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();
}

void rlib_wake(uint64_t *pointer, uint64_t value, uint64_t count) {
    comm_t *schedin;
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));

    sched_in_packet_t in;
    in.type = SCHED_WAKE;
    in.req_id = rlib_sequence();
    in.wake.address = (uint64_t)pointer;
    in.wake.value = (uint64_t)value;
    in.wake.count = count;
    comm_write(schedin, &in, sizeof(in));
    comm_flush(schedin);

    rlib_process_queued();
}

void rlib_process_queued() {
    uint64_t own_id;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("int $0xfe" : : "a"(own_id));
}

void rlib_yield() {
    __asm__ __volatile__("int $0xff");
}
