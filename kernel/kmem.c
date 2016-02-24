#include "kmem.h"
#include "kutil.h"

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

        if(!ok) return 0;
        
        uint64_t entry = phy_read64(prev);

        if((entry & 1) == 0) {
            *ok = 0;
            return 0;
        }

        entry &= ~0xfff;

        return entry + index * 8;
    }
}

uint64_t kmem_boot() {
    return boot_cr3;
}

void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint16_t flags) {
    uint8_t ok;
    // try short version
    uint64_t addr = kmem_paging_addr(root, vaddr, 3, &ok);
    if(!ok) {
        // need to allocate some intermediate entries
        uint64_t pa = -1;
        for(int i = 0; i <= 3; i ++) {
            uint64_t a = kmem_paging_addr(root, vaddr, i, &ok);
            if(!ok) {
                uint64_t ntable = kmem_getpage();
                phy_write64(pa, ntable | 0x7);
                a = kmem_paging_addr(root, vaddr, i, &ok);
            }
            pa = a;
        }

        addr = kmem_paging_addr(root, vaddr, 3, &ok);
    }

    phy_write64(addr, page | flags);
}
