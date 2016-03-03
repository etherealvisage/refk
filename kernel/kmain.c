#include <stdint.h>

#include "kutil.h"
#include "kmem.h"
#include "task.h"

#include "images/transfer.h"
#include "images/boot.h"

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
        memcpy((void *)TASK_BASE, images_transfer, images_transfer_len);
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
        TASK_MEM(0)->valid = 1;
    }

    // clear the main screen
    for(int i = 0; i < 80*24*2; i ++) {
        phy_write8(0xb8000 + i, 0);
    }

    void (*transfer)(void *, void *) = (void *)0xffffffffffe00000;

    task_state_t *ntask = TASK_MEM(1);

    uint64_t nroot = kmem_create_root();

    ntask->cs = 0x08;
    ntask->ds = 0x10;
    ntask->ss = 0x10;
    ntask->rflags = 0x46;
    ntask->rip = (uint64_t)transfer;
    ntask->rsp = TASK_ADDR(3);
    ntask->rdi = TASK_ADDR(1);
    ntask->rsi = TASK_ADDR(0);
    ntask->cr3 = nroot;

    transfer(TASK_MEM(1), ntask);

    d_printf("transferred back!\n");

    TASK_MEM(1)->valid = 1;

    task_state_t *task = task_create(boot_elf, 0x10000);

    d_printf("new task address: %x\n", task);
    transfer(TASK_MEM(1), task);

    // should never be reached!
    while(1) {}
}
