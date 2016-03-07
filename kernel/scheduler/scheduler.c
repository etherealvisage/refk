#include "klib/kutil.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/kcomm.h"
#include "klib/lapic.h"
#include "klib/kmem.h"

#include "scheduler.h"

char tick_stack[1024];

static void tick(uint64_t vector, uint64_t excode, task_state_t *ret_task) {
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
    else {
        d_printf("Not in runnable task!\n");
    }

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

char spawner_stack[1024];

static void spawner() {
    __asm__("sti");
    d_printf("Spawning!\n");

    kcomm_t *schedin = (void *)COMM_BASE_ADDRESS;
    kcomm_t *schedout = (void *)COMM_BASE_ADDRESS + COMM_OUT_OFFSET;

    /* Spawn t1 thread */
    comm_in_packet_t in;
    in.type = COMM_SPAWN;
    in.spawn.cr3 = kmem_current();

    kcomm_put(schedin, &in, sizeof(in));

    comm_out_packet_t out;
    uint64_t result_size;
    while(kcomm_get(schedout, &out, &result_size)) ;

    task_state_t *ts = out.spawn.task;
    in.type = COMM_SET_STATE;
    in.set_state.state = ts;
#define SET(index_, value_) \
    in.set_state.index = index_; \
    in.set_state.value = (uint64_t)value_; \
    kcomm_put(schedin, &in, sizeof(in));

    SET(COMM_STATE_RIP, t1)
    SET(COMM_STATE_RSP, t1_stack + 1024)
    SET(COMM_STATE, TASK_STATE_VALID | TASK_STATE_RUNNABLE);

    d_printf("Spawned t1 thread.\n");

    /* Spawn t2 thread */
    in.type = COMM_SPAWN;
    in.spawn.cr3 = kmem_current();

    kcomm_put(schedin, &in, sizeof(in));

    while(kcomm_get(schedout, &out, &result_size)) ;

    ts = out.spawn.task;
    in.type = COMM_SET_STATE;
    in.set_state.state = ts;

    SET(COMM_STATE_RIP, t2)
    SET(COMM_STATE_RSP, t2_stack + 1024)
    SET(COMM_STATE, TASK_STATE_VALID | TASK_STATE_RUNNABLE);

    d_printf("Spawned t2 thread.\n");

    while(1) {}
}

void listen() {
    d_printf("Listening!\n");
    kcomm_t *schedin = (void *)COMM_BASE_ADDRESS;
    kcomm_t *schedout = (void *)COMM_BASE_ADDRESS + COMM_OUT_OFFSET;

    comm_in_packet_t in;
    while(1) {
        uint64_t in_size;
        if(kcomm_get(schedin, &in, &in_size)) continue;

        switch(in.type) {
        case COMM_FORWARD: {
            break;
        }
        case COMM_SPAWN: {
            task_state_t *ts = task_create();
            ts->cr3 = in.spawn.cr3;
            comm_out_packet_t out;
            out.type = COMM_SPAWN;
            out.spawn.task = ts;
            kcomm_put(schedout, &out, sizeof(out));
            break;
        }
        case COMM_SET_STATE: {
            task_state_t *ts = in.set_state.state;
            uint64_t *indexed = (void *)ts;
            indexed[in.set_state.index] = in.set_state.value;
            break;
        }
        default:
            d_printf("Unknown comm_in packet type!\n");
            break;
        }
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

    DESC_INT_TASKS_MEM[0x82] = (uint64_t)tick_ts;

    task_state_t *spawner_task = task_create();
    task_set_local(spawner_task, spawner, spawner_stack + 1024);
    task_mark_runnable(spawner_task);

    task_state_t *self_task = task_create();
    self_task->state = TASK_STATE_VALID | TASK_STATE_RUNNABLE;
    //task_mark_runnable(self_task);

    __asm__("sti");
    
    void (*transfer)(task_state_t *, task_state_t *) = (void *)0xffffffffffe00000;
    transfer(self_task, self_task);

    listen();

    while(1) {}
}
