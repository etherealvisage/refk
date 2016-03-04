#include "klib/kutil.h"
#include "klib/kmem.h"
#include "klib/task.h"
#include "klib/desc.h"

#include "lapic.h"
#include "ioapic.h"

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

char test_stack[1024];

void _start() {
    lapic_init();

    ioapic_init();

    {
        task_state_t *nt = TASK_MEM(5);

        nt->rflags = 0x46;
        nt->cs = 0x08;
        nt->ds = 0x10;
        nt->ss = 0x10;
        nt->rip = (uint64_t)irq_handler;
        nt->rsp = (uint64_t)test_stack + 1024;
        nt->valid = 1;
        nt->cr3 = kmem_current();

        for(int i = 0x80; i <= 0x98; i ++) {
            DESC_INT_TASKS_MEM[i] = 5;
        }
    }

    d_printf("About to trigger interrupts...\n");

    __asm__ __volatile__("int3");

    d_printf("After interrupt!\n");

    __asm__("sti");

    // TODO: delete task
    while(1) {}
}
