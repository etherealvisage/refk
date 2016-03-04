#include "kmem.h"
#include "klib/kutil.h"
#include "klib/task.h"
#include "klib/desc.h"

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

uint64_t kmem_boot() {
    return boot_cr3;
}
