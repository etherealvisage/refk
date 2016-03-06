#include "klib/kutil.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/kcomm.h"
#include "klib/lapic.h"

#include "scheduler.h"

char tick_stack[1024];

static void tick(uint64_t vector, uint64_t excode, task_state_t *ret_task) {
    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;

    d_printf("tick!\n");

    if(ret_task->state & TASK_STATE_RUNNABLE) {
        d_printf("Previous task was runnable, so should swap to new runnable!\n");

        d_printf("ret_task: %x\n", ret_task);
        d_printf("TASK_MEM(0): %x\n", TASK_MEM(0));
        int64_t init = ret_task - TASK_MEM(0);

        d_printf("Originally at index %x\n", init);

        task_state_t *nts = ret_task;
        for(int off = 1; off < NUM_TASKS; off ++) {
            int in = (init + off) % NUM_TASKS;
            if(in == 0) continue; // skip task 0
            if((TASK_MEM(in)->state & (TASK_STATE_VALID | TASK_STATE_RUNNABLE))
                != (TASK_STATE_VALID | TASK_STATE_RUNNABLE)) {
                
                continue;
            }

            d_printf("Switching to index %x\n", in);
            nts = TASK_MEM(in);
            break;
        }

        ret_task = nts;
    }

    d_printf("Switching to task %x\n", ret_task);

    lapic_conditional_eoi(vector);
    transfer(0, ret_task);
}

char t1_stack[1024];

static void t1() {
    __asm__("sti");
    while(1) {
        d_printf("Thread #1!\n");
        for(int i = 0; i < 10000000; i ++) {}
    }
}

char t2_stack[1024];

static void t2() {
    __asm__("sti");
    while(1) {
        d_printf("Thread #2!\n");
        for(int i = 0; i < 10000000; i ++) {}
    }
}

void _start() {
    d_printf("scheduler!\n");
    lapic_setup();

    kcomm_t *schedin = (void *)COMM_BASE_ADDRESS;
    kcomm_t *schedout = (void *)COMM_BASE_ADDRESS + COMM_OUT_OFFSET;

    uint64_t data, len;

    if(!kcomm_get(schedin, &data, &len)) {
        d_printf("Got data: %x\n", data);
    }

    task_state_t *tick_ts = task_create();
    task_set_local(tick_ts, tick, tick_stack + 1024);

    d_printf("setting IRQ for 0x82 to 0x%x\n", tick_ts);
    DESC_INT_TASKS_MEM[0x82] = (uint64_t)tick_ts;

    task_state_t *t1_task = task_create();
    task_set_local(t1_task, t1, t1_stack + 1024);
    task_mark_runnable(t1_task);
    task_state_t *t2_task = task_create();
    task_set_local(t2_task, t2, t2_stack + 1024);
    task_mark_runnable(t2_task);

    __asm__("sti");
    
    void (*transfer)(uint64_t, task_state_t *) = (void *)0xffffffffffe00000;
    transfer(0, t1_task);

    while(1) {}
}
