#include "klib/kutil.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/kcomm.h"
#include "klib/lapic.h"
#include "klib/kmem.h"
#include "klib/sheap.h"
#include "klib/avl.h"

#include "mman.h"
#include "task.h"
#include "listen.h"
#include "synch.h"

static task_state_t *choose_next(task_state_t *current) {
    int64_t init = current - TASK_MEM(0);

    task_state_t *nts;
    if(current->state & TASK_STATE_BLOCKED) nts = 0;
    else nts = current;

    for(int off = 1; off < NUM_TASKS; off ++) {
        int in = (init + off) % NUM_TASKS;
        if(in == 0) continue; // skip task 0

        const uint64_t wanted_value =
            TASK_STATE_VALID | TASK_STATE_RUNNABLE;
        const uint64_t wanted_mask =
            TASK_STATE_VALID | TASK_STATE_RUNNABLE | TASK_STATE_BLOCKED;

        if((TASK_MEM(in)->state & wanted_mask) != wanted_value) {                
            continue;
        }

        nts = TASK_MEM(in);
        break;
    }

    return nts;
}

char change_stack[1024];
static void change_task(uint64_t vector,
    uint64_t __attribute__((unused)) excode, task_state_t *ret_task) {

    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;

    if((ret_task->state & TASK_STATE_RUNNABLE)) {
        ret_task = choose_next(ret_task);
    }

    lapic_conditional_eoi(vector);
    transfer(0, ret_task);
}

char process_stack[4096];
static void process_queue(uint64_t __attribute__((unused)) vector,
    uint64_t __attribute__((unused)) excode, task_state_t *ret_task) {

    process_for(ret_task->rax);

    // if the current task just became blocked, choose a new one
    if(ret_task->state & TASK_STATE_BLOCKED) {
        ret_task = choose_next(ret_task);
    }

    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;
    transfer(0, ret_task);
}

void _start(uint64_t bootproc_cr3, task_state_t *hw_task) {
    d_printf("scheduler!\n");
    sheap_init();

    // init local components
    mman_init(bootproc_cr3);
    task_init();
    synch_init();

    task_state_t *tick_ts = task_create();
    task_set_local(tick_ts, change_task, change_stack + 1024);

    DESC_INT_TASKS_MEM[0xff] = (uint64_t)tick_ts;

    task_state_t *process_ts = task_create();
    task_set_local(process_ts, process_queue, process_stack + 4096);
    DESC_INT_TASKS_MEM[0xfe] = (uint64_t)process_ts;

    // the scheduler uses task #1.
    TASK_MEM(1)->state = TASK_STATE_VALID | TASK_STATE_RUNNABLE;

    listen(hw_task);

    // should never be reached
    while(1) {}
}
