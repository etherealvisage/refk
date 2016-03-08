#include "klib/kutil.h"
#include "klib/kmem.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/lapic.h"
#include "klib/kcomm.h"
#include "ioapic.h"

#include "../scheduler/interface.h"

const uint8_t scheduler_image[] = {
#include "../images/scheduler.h"
};

void _start() {
    kmem_setup();

    lapic_init();
    lapic_setup();

    // TODO: perform early ACPI tables read for I/O APIC information

    ioapic_init();

    d_printf("boot initialization completed\n");

    // use TASK_MEM(1) for scheduler task
    task_load_elf(TASK_MEM(1), scheduler_image, 0x10000);
    // pass in the root CR3 for the boot process
    TASK_MEM(1)->rdi = kmem_current();
    /* TODO: spawn hardware task */
    // TODO: pass in the root CR3 for the hardware process

    // remove the current thread and swap to the scheduler
    void (*transfer)(void *, void *) = (void *)0xffffffffffe00000;
    transfer(0, TASK_MEM(1));

    // TODO: delete task
    while(1) {}
}
