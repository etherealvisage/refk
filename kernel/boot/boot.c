#include "klib/kutil.h"
#include "klib/kmem.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/lapic.h"
#include "ioapic.h"

#include "../images/scheduler.h"

void test(uint64_t vector, uint64_t excode, uint64_t ret_task) {
    void (*transfer)(uint64_t, uint64_t) = (void *)0xffffffffffe00000;

    transfer(0, ret_task);
    while(1) {}
}

void irq_handler(uint64_t vector, uint64_t excode, uint64_t ret_task) {
    void (*transfer)(uint64_t, uint64_t) = (void *)0xffffffffffe00000;

    uint64_t irq = vector - 0x80;
    if(irq == 0x1) {
        uint8_t key = kinb(0x60);
        d_printf("key pressed: %x\n", key);
        // ack receipt
        uint8_t a = kinb(0x61);
        a |= 0x82;
        koutb(0x61, a);
        a &= 0x7f;
        koutb(0x61, a);
    }

    lapic_conditional_eoi(vector);

    transfer(0, ret_task);
    while(1) {}
}

char irq_stack[1024];

void _start() {
    kmem_setup();

    lapic_init();
    lapic_setup();

    ioapic_init();

    /* TODO: spawn hardware task */

    /* TODO: spawn scheduler task */
    // XXX: this relies on behaviour of the task_create function to pick the
    // numerically-lowest available ID!
    TASK_MEM(1)->state &= ~TASK_STATE_VALID;

    // remove the current thread and swap to the scheduler
    // TODO: release the memory associated with this task!
    task_state_t *ts = task_create(scheduler_elf, 0x10000);
    void (*transfer)(uint64_t, uint64_t) = (void *)0xffffffffffe00000;
    transfer(0, (uint64_t)ts);

    /*{
        d_printf("state: %x\n", TASK_MEM(0)->state);
        task_state_t *irqts = task_create_local(irq_handler, irq_stack + 1024);
        d_printf("irqts: %x\n", irqts);
        for(int i = 0x80; i <= 0x98; i ++) {
            DESC_INT_TASKS_MEM[i] = (uint64_t)irqts;
        }
    }*/

    __asm__("sti");

    // TODO: delete task
    while(1) {}
}
