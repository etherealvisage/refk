#include <stdint.h>

#include "kutil.h"
#include "kmem.h"
#include "task.h"

void test() {
    d_printf("test\n");
    while(1) {}
}

void kmain(uint64_t *mem) {
    d_init(); // set up the serial port
    kmem_init(mem); // initialize bootstrap memory manager

    // clear the main screen
    for(int i = 0; i < 80*24*2; i ++) {
        phy_write8(0xb8000 + i, 0);
    }

    void (*transfer)(void *, void *) = (void *)0xffffffffffe00000;

    char smallstack[256];

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

    transfer(TASK_MEM(0), ntask);

    d_printf("transferred back!\n");

    // should never be reached!
    while(1) {}
}
