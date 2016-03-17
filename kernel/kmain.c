#include <stdint.h>

#include "clib/mem.h"

#include "klib/task.h"
#include "klib/phy.h"

#include "d.h"
#include "kmem.h"
#include "desc.h"
#include "status.h"

const uint8_t transfer_image[] = {
#include "images/transfer.h"
};

const uint8_t boot_image[] = {
#include "images/boot.h"
};

const uint8_t scheduler_image[] = {
#include "images/scheduler.h"
};

const uint8_t hw_image[] = {
#include "images/hw.h"
};

void kmain(uint64_t *mem) {
    d_init(); // set up the serial port
    d_printf("Rebooting...\n");
    kmem_init(mem); // initialize bootstrap memory manager

    // task initialization
    {
        uint64_t transfer_page = kmem_getpage();
        // map as data initially
        kmem_map(kmem_boot(), TASK_BASE, transfer_page,
            KMEM_MAP_DEFAULT);
        mem_copy((void *)TASK_BASE, transfer_image, sizeof(transfer_image));
        // remap as code
        kmem_map(kmem_boot(), TASK_BASE, transfer_page,
            KMEM_MAP_CODE);

        /* map task memory */
        uint64_t ptr = TASK_BASE + 0x1000;
        for(int i = 0; i < NUM_TASKS; i += 16) {
            uint64_t page = kmem_getpage();
            kmem_map(kmem_boot(), ptr, page, KMEM_MAP_DEFAULT);

            ptr += 0x1000;
        }
        // clear task memory
        mem_set((void *)(TASK_BASE + 0x1000), 0, NUM_TASKS * 256);

        // mark task #0 as valid, this will be used as temporary stack space
        // by the switcher
        TASK_MEM(0)->state = TASK_STATE_VALID;
    }
    desc_init();

    // clear the main screen
    for(int i = 0; i < 80*24*2; i ++) {
        phy_write8(0xb8000 + i, 0);
    }

    // create status page
    kmem_map(kmem_current(), STATUS_BASE, kmem_getpage(), KMEM_MAP_RO_DATA);

    void (*transfer)(void *, void *) = (void *)0xffffffffffe00000;

    // use TASK_MEM(1) for scheduler task
    task_state_t *schts = task_create();
    *TASK_MEM(1) = *schts;
    schts->state = 0;
    schts = TASK_MEM(1);
    task_load_elf(TASK_MEM(1), scheduler_image, 0x10000);
    // pass in the root CR3 for the boot process
    TASK_MEM(1)->rdi = kmem_current();

    // use TASK_MEM(2) for hw task
    task_state_t *hwts = task_create();
    *TASK_MEM(2) = *hwts;
    hwts->state = 0;
    hwts = TASK_MEM(2);
    task_load_elf(hwts, hw_image, 0x10000);

    // pass in the task state for the hw thread into the scheduler
    TASK_MEM(1)->rsi = (uint64_t)TASK_MEM(2);

    // remove the current thread and swap to the scheduler
    transfer(0, TASK_MEM(1));

    // should never be reached!
    while(1) {}
}
