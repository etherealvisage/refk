#include "kmem.h"
#include "kutil.h"
#include "task.h"
#include "desc.h"

extern char kernel_pbase;
extern char _data_end;

uint64_t last_unused;
uint64_t boot_cr3;

void kmem_init(uint64_t *regions) {
    // perform initial pass to round region start/end as appropriate
    for(int i = 0; regions[i] || regions[i+1]; i += 2) {
        uint64_t start = regions[i];
        uint64_t end = regions[i] + regions[i+1];
        // round the start page up
        start = (start+0xfff) & ~0xfff;
        // round the end page down
        end &= ~0xfff;

        regions[i] = start;
        // NOTE: unless starting size was negative, will not be negative,
        // though it may be zero.
        regions[i+1] = end - start;
    }


    // use sentinel value for last_unused
    last_unused = -1ul;
    // mark all pages as unused
    uint64_t kernel_start = (uint64_t)(&kernel_pbase);
    uint64_t kernel_end = (uint64_t)(&_data_end);
    for(int i = 0; regions[i] || regions[i+1]; i += 2) {
        const uint64_t end = regions[i] + regions[i+1];
        for(uint64_t p = regions[i]; p < end; p += 0x1000) {
            // don't mark kernel load pages as unused
            if(p >= kernel_start && p <= kernel_end) continue;
            // don't use first 2MB
            if(p <= 0x20000) continue;

            kmem_unuse(p);
        }
    }

    // get boot CR3 value
    __asm__ __volatile__ ("mov rax, cr3" : "=a"(boot_cr3));

    // set up basic images
    desc_init();
}

void kmem_unuse(uint64_t page) {
    phy_write64(page, last_unused);
    last_unused = page;
}

uint64_t kmem_getpage() {
    // TODO: handle error conditions!
    uint64_t ret = last_unused;
    last_unused = phy_read64(ret);
    return ret;
}

static uint64_t kmem_paging_addr(uint64_t root, uint64_t address,
    uint8_t level, uint8_t *ok) {

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
                phy_write64(pa, ntable | 0x7);
                a = kmem_paging_addr(root, vaddr, i, &ok);
            }
            pa = a;
        }

        addr = kmem_paging_addr(root, vaddr, level, &ok);
    }

    return addr;
}

uint64_t kmem_boot() {
    return boot_cr3;
}

uint64_t kmem_current() {
    uint64_t current;
    __asm__ __volatile__ ("mov rax, cr3" : "=a"(current));
    return current;
}

uint64_t kmem_create_root() {
    uint64_t ret = kmem_getpage();

    // zero out root
    for(int i = 0; i < 512; i ++) phy_write64(ret + i*8, 0);

    // add physical memory map
    // this uses 1GB pages, so want entry in level 1 (that covers phy and more)
    uint64_t nentry = kmem_paging_addr_create(ret, 0xffffc00000000000ULL, 1);
    uint64_t bentry =
        kmem_paging_addr_create(boot_cr3, 0xffffc00000000000ULL, 1);
    phy_write64(nentry, phy_read64(bentry));
    // copy task level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, TASK_BASE, 2);
    bentry = kmem_paging_addr_create(boot_cr3, TASK_BASE, 2);
    phy_write64(nentry, phy_read64(bentry));
    // copy descriptors level 2 structure (2MB total)
    nentry = kmem_paging_addr_create(ret, DESC_BASE, 2);
    bentry = kmem_paging_addr_create(boot_cr3, DESC_BASE, 2);
    phy_write64(nentry, phy_read64(bentry));

    /*{
        d_printf("comparison of values:\n");
        uint64_t e0 = kmem_paging_addr_create(ret, TASK_BASE, 0);
        uint64_t e1 = kmem_paging_addr_create(ret, TASK_BASE, 1);
        uint64_t e2 = kmem_paging_addr_create(ret, TASK_BASE, 2);
        uint64_t e3 = kmem_paging_addr_create(ret, TASK_BASE, 3);
        d_printf("nentry: %x %x %x %x\n", e0, e1, e2, e3);
        e0 = phy_read64(e0);
        e1 = phy_read64(e1);
        e2 = phy_read64(e2);
        e3 = phy_read64(e3);
        d_printf("nentry *: %x %x %x %x\n", e0, e1, e2, e3);
        e0 = kmem_paging_addr_create(boot_cr3, TASK_BASE, 0);
        e1 = kmem_paging_addr_create(boot_cr3, TASK_BASE, 1);
        e2 = kmem_paging_addr_create(boot_cr3, TASK_BASE, 2);
        e3 = kmem_paging_addr_create(boot_cr3, TASK_BASE, 3);
        d_printf("bentry: %x %x %x %x\n", e0, e1, e2, e3);
        e0 = phy_read64(e0);
        e1 = phy_read64(e1);
        e2 = phy_read64(e2);
        e3 = phy_read64(e3);
        d_printf("bentry *: %x %x %x %x\n", e0, e1, e2, e3);
    }*/

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
        if(!ok) return;
        uint64_t phy_addr = phy_read64(entry) & ~KMEM_FLAG_MASK;

        uint64_t wsize = 0x1000 - pg_off;
        if(wsize > size) wsize = size;

        memcpy((void *)(0xffffc00000000000ULL + phy_addr + pg_off),
            data, wsize);

        vaddr += wsize;
        size -= wsize;
    }
}
