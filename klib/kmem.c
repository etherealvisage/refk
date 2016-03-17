#include "clib/mem.h"

#include "kmem.h"
#include "kutil.h"
#include "task.h"
#include "desc.h"

#include "kernel/status.h"

uint64_t temp_last;
uint64_t *last_unused = &temp_last;

void kmem_setup() {
    last_unused = (uint64_t *)KMEM_BASE_ADDR;
}

void kmem_setup_bootstrap(uint64_t last) {
    kmem_setup();
    *last_unused = last;
}

void kmem_unuse(uint64_t page) {
    phy_write64(page, *last_unused);
    *last_unused = page;
}

uint64_t kmem_getpage() {
    // TODO: handle error conditions!
    uint64_t ret = *last_unused;
    *last_unused = phy_read64(ret);
    return ret;
}

uint64_t kmem_paging_addr(uint64_t root, uint64_t address, uint8_t level,
    uint8_t *ok) {

    // maps level -> shift
    //          0 -> 39
    //          1 -> 30
    //          2 -> 21
    //          3 -> 12
    uint64_t index = (address >> (12 + (3-level)*9)) & 0x1ff;

    if(level == 0) {
        *ok = 1;
        return root + index * 8;
    }
    else {
        uint64_t prev = kmem_paging_addr(root, address, level-1, ok);

        if(!*ok) return 0;

        uint64_t entry = phy_read64(prev);

        if((entry & 1) == 0) {
            *ok = 0;
            return 0;
        }

        entry &= ~0xfff;

        return entry + index * 8;
    }
}

static uint64_t kmem_paging_addr_create(uint64_t root, uint64_t vaddr,
    uint8_t level) {

    uint8_t ok;
    uint64_t addr = kmem_paging_addr(root, vaddr, level, &ok);

    if(!ok) {
        // need to allocate some intermediate entries
        uint64_t pa = -1;
        for(int i = 0; i <= level; i ++) {
            uint64_t a = kmem_paging_addr(root, vaddr, i, &ok);
            if(!ok) {
                uint64_t ntable = kmem_getpage();
                for(int i = 0; i < 512; i ++) phy_write64(ntable + i*8, 0);
                phy_write64(pa, ntable | 0x7);
                a = kmem_paging_addr(root, vaddr, i, &ok);
            }
            pa = a;
        }

        addr = kmem_paging_addr(root, vaddr, level, &ok);
    }

    return addr;
}

uint64_t kmem_current() {
    uint64_t current = 0;
    __asm__ __volatile__ ("mov %%cr3, %%rax" : "=a"(current));
    return current;
}

uint64_t kmem_create_root() {
    uint64_t ret = kmem_getpage();

    // zero out root
    for(int i = 0; i < 512; i ++) phy_write64(ret + i*8, 0);

    // add physical memory map
    // this uses 1GB pages, so want entry in level 0 (that covers phy and more)
    uint64_t nentry = kmem_paging_addr_create(ret, 0xffffc00000000000ULL, 0);
    uint64_t bentry =
        kmem_paging_addr_create(kmem_current(), 0xffffc00000000000ULL, 0);
    phy_write64(nentry, phy_read64(bentry));
    // copy task level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, TASK_BASE, 2);
    bentry = kmem_paging_addr_create(kmem_current(), TASK_BASE, 2);
    phy_write64(nentry, phy_read64(bentry));
    // copy descriptors level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, DESC_BASE, 2);
    bentry = kmem_paging_addr_create(kmem_current(), DESC_BASE, 2);
    phy_write64(nentry, phy_read64(bentry));
    // copy memory manager level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, KMEM_BASE_ADDR, 2);
    bentry = kmem_paging_addr_create(kmem_current(), KMEM_BASE_ADDR, 2);
    phy_write64(nentry, phy_read64(bentry));
    // copy status page level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, STATUS_BASE, 2);
    bentry = kmem_paging_addr_create(kmem_current(), STATUS_BASE, 2);
    phy_write64(nentry, phy_read64(bentry));

    return ret;
}

void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint64_t flags) {
    uint64_t addr = kmem_paging_addr_create(root, vaddr, 3);
    phy_write64(addr, page | flags);
}

void kmem_set_flags(uint64_t root, uint64_t vaddr, uint64_t flags) {
    uint64_t addr = kmem_paging_addr_create(root, vaddr, 3);
    phy_write64(addr, (phy_read64(addr) & ~KMEM_FLAG_MASK) | flags);
}

void kmem_memcpy(uint64_t root, uint64_t vaddr, void *data, uint64_t size) {
    uint8_t ok;
    // NOTE: this assumes 4KB pages!
    while(size > 0) {
        uint64_t pg = vaddr & ~0xfff;
        uint64_t pg_off = vaddr & 0xfff;

        uint64_t entry = kmem_paging_addr(root, pg, 3, &ok);
        if(!ok) {
            return;
        }
        uint64_t phy_addr = phy_read64(entry) & ~KMEM_FLAG_MASK;

        uint64_t wsize = 0x1000 - pg_off;
        if(wsize > size) wsize = size;

        mem_copy((void *)(0xffffc00000000000ULL + phy_addr + pg_off),
            data, wsize);

        data += wsize;
        vaddr += wsize;
        size -= wsize;
    }
}

void kmem_memclr(uint64_t root, uint64_t vaddr, uint64_t size) {
    uint8_t ok;
    // NOTE: this assumes 4KB pages!
    while(size > 0) {
        uint64_t pg = vaddr & ~0xfff;
        uint64_t pg_off = vaddr & 0xfff;

        uint64_t entry = kmem_paging_addr(root, pg, 3, &ok);
        if(!ok) {
            return;
        }
        uint64_t phy_addr = phy_read64(entry) & ~KMEM_FLAG_MASK;

        uint64_t wsize = 0x1000 - pg_off;
        if(wsize > size) wsize = size;

        mem_set((void *)(0xffffc00000000000ULL + phy_addr + pg_off), 0, wsize);

        vaddr += wsize;
        size -= wsize;
    }
}
