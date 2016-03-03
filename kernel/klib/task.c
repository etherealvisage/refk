#include "task.h"
#include "klib/elf.h"
#include "klib/kmem.h"
#include "kutil.h"

#include "images/transfer.h"

void task_init() {
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
}

task_state_t *task_create(void *elf_image) {
    task_state_t *ts = 0;
    for(int i = 0; i < NUM_TASKS; i ++) {
        task_state_t *t = TASK_MEM(i);
        if(t->valid) continue;
        ts = t;
        break;
    }

    if(!ts) return 0;

    ts->cr3 = kmem_create_root();

    return ts;
}
