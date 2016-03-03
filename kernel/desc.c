#include "desc.h"
#include "kmem.h"

static void gdt_set_null(uint64_t index) {
    uint64_t *gdt_memory = (uint64_t *)DESC_GDT_ADDR;
    gdt_memory[index] = 0;
}

static void gdt_set_code(uint64_t index, uint8_t dpl) {
    uint64_t *gdt_memory = (uint64_t *)DESC_GDT_ADDR;
    gdt_memory[index] = 0;
    // set dpl
    gdt_memory[index] |= ((uint64_t)dpl << (13+32));
    // set P (present) flag
    gdt_memory[index] |= 1ULL<<(15+32);
    // C flag is zero.
    // L flag is one; we want 64-bit code.
    gdt_memory[index] |= 1ULL<<(21+32);
    // D flag is zero, as the L flag is one.
    
    // set type
    gdt_memory[index] |= 3ULL<<(11+32);

}

static void gdt_set_data(uint64_t index) {
    uint64_t *gdt_memory = (uint64_t *)DESC_GDT_ADDR;
    gdt_memory[index] = 0;
    // set P (present) flag
    gdt_memory[index] |= 1ULL<<(15+32);
    // set type
    gdt_memory[index] |= 2ULL<<(11+32);
    // set writable
    gdt_memory[index] |= 1ULL<<(9+32);
}

void desc_init() {
    uint64_t gdt_page = kmem_getpage();
    kmem_map(kmem_boot(), DESC_GDT_ADDR, gdt_page, KMEM_MAP_DEFAULT);

    gdt_set_null(0);
    gdt_set_code(1, 0);
    gdt_set_data(2);
    gdt_set_code(3, 3);

    /* load new GDT */
    {
        uint8_t gdt_pointer[10];
        *(uint16_t *)gdt_pointer = 0xffff;
        *(uint64_t *)(gdt_pointer+2) = DESC_GDT_ADDR;
        __asm__ __volatile__("lgdt [rax]" : : "a"(&gdt_pointer));
    }
}
