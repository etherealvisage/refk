#include <stdint.h>

#include "kutil.h"
#include "kmem.h"
#include "klib/task.h"
#include "desc.h"

const uint8_t transfer_image[] = {
#include "images/transfer.h"
};

const uint8_t boot_image[] = {
#include "images/boot.h"
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
        memcpy((void *)TASK_BASE, transfer_image, sizeof(transfer_image));
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

        // mark task #0 as valid, this will be used as temporary stack space
        // by the switcher
        TASK_MEM(0)->state = TASK_STATE_VALID;
    }
    desc_init();

    // clear the main screen
    for(int i = 0; i < 80*24*2; i ++) {
        phy_write8(0xb8000 + i, 0);
    }

    void (*transfer)(void *, void *) = (void *)0xffffffffffe00000;

    task_state_t *task = task_create(boot_image, 0x10000);

    transfer(0, task);

    // should never be reached!
    while(1) {}
}
