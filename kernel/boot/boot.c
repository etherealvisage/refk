#include "klib/kutil.h"
#include "klib/kmem.h"
#include "klib/task.h"

#include "lapic.h"
#include "../desc.h"

void test(uint64_t vector, uint64_t excode, uint64_t ret_task) {
    void (*transfer)(uint64_t, uint64_t) = (void *)0xffffffffffe00000;

    transfer(0, ret_task);
    while(1) {}
}

char test_stack[1024];

void _start() {
    lapic_init();

    task_state_t *nt = TASK_MEM(5);

    nt->rflags = 0x46;
    nt->cs = 0x08;
    nt->ds = 0x10;
    nt->ss = 0x10;
    nt->rip = (uint64_t)test;
    nt->rsp = (uint64_t)test_stack + 1024;
    nt->valid = 1;
    nt->cr3 = kmem_current();

    DESC_INT_TASKS_MEM[3] = 5;

    d_printf("About to trigger interrupts...\n");

    __asm__ __volatile__("int3");

    d_printf("After interrupt!\n");

    // TODO: delete task
    while(1) {}
}
