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

char tick_stack[1024];

static void tick(uint64_t vector, uint64_t __attribute__((unused)) excode,
    task_state_t *ret_task) {

    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;

    if(ret_task->state & TASK_STATE_RUNNABLE) {
        int64_t init = ret_task - TASK_MEM(0);

        task_state_t *nts = ret_task;
        for(int off = 1; off < NUM_TASKS; off ++) {
            int in = (init + off) % NUM_TASKS;
            if(in == 0) continue; // skip task 0
            if((TASK_MEM(in)->state & (TASK_STATE_VALID | TASK_STATE_RUNNABLE))
                != (TASK_STATE_VALID | TASK_STATE_RUNNABLE)) {
                
                continue;
            }

            nts = TASK_MEM(in);
            break;
        }

        ret_task = nts;
    }

    lapic_conditional_eoi(vector);
    transfer(0, ret_task);
}

char process_stack[4096];

static void process(uint64_t vector, uint64_t __attribute__((unused)) excode,
    task_state_t *ret_task) {

    process_for(ret_task->rax);

    uint64_t xor = 0;
    for(int i = 0; i < 32; i ++) {
        xor ^= ((uint64_t *)ret_task)[i];
    }

    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;
    transfer(0, ret_task);
}

void _start(uint64_t bootproc_cr3, task_state_t *hw_task) {
    d_printf("scheduler!\n");
    //lapic_setup();
    sheap_init();

    // init local components
    mman_init(bootproc_cr3);
    task_init();


    task_state_t *tick_ts = task_create();
    task_set_local(tick_ts, tick, tick_stack + 1024);

    DESC_INT_TASKS_MEM[0xff] = (uint64_t)tick_ts;

    task_state_t *process_ts = task_create();
    task_set_local(process_ts, process, process_stack + 4096);
    DESC_INT_TASKS_MEM[0xfe] = (uint64_t)process_ts;

    // the scheduler uses task #1.
    TASK_MEM(1)->state = TASK_STATE_VALID | TASK_STATE_RUNNABLE;

    listen(hw_task);

    while(1) {}
}
