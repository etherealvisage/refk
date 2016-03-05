#include "desc.h"
#include "kmem.h"

#include "kutil.h"

const uint8_t intr_image[] = {
#include "images/intr.h"
};

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

static void idt_set(uint64_t index, uint64_t entry, uint8_t dpl, uint8_t ist) {
    uint64_t *idt = (uint64_t *)DESC_IDT_ADDR;
    // clear IDT entries
    idt[index*2] = 0;
    idt[index*2 + 1] = 0;

    // lower 16 bits of entry point
    idt[index*2] = entry & 0xffff;
    // entry point CS selector (0x08 = default CS)
    idt[index*2] |= 0x08 << 16;
    // interruintpt stack table
    idt[index*2] |= (ist & 0x7ULL) << 32;
    // type (0xe is interruintpt gate, by Table 3-2 in Intel vol 3A)
    idt[index*2] |= 0xeULL << (32 + 8);
    // DPL
    idt[index*2] |= (dpl & 0x03ULL) << (32 + 13);
    // present
    idt[index*2] |= 1ULL << (32+15);
    // entry offset second 16 bits
    idt[index*2] |= ((entry >> 16) & 0xffff) << (32 + 16);
    // entry offset uintpper 32 bits
    idt[index*2 + 1] |= (entry >> 32) & 0xffffffff;
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

    uint64_t idt_page = kmem_getpage();
    kmem_map(kmem_boot(), DESC_IDT_ADDR, idt_page, KMEM_MAP_DEFAULT);
    uint64_t tasks_page = kmem_getpage();
    kmem_map(kmem_boot(), DESC_INT_TASKS_ADDR, tasks_page, KMEM_MAP_DEFAULT);
    // map intr pages
    for(uint64_t i = 0; i < sizeof(intr_image); i += 0x1000) {
        kmem_map(kmem_boot(), DESC_INT_CODE_ADDR + i, kmem_getpage(),
            KMEM_MAP_DEFAULT);
    }

    memcpy((void *)DESC_INT_CODE_ADDR, intr_image, sizeof(intr_image));
    // protect intr pages
    for(uint64_t i = 0; i < sizeof(intr_image); i += 0x1000) {
        kmem_set_flags(kmem_boot(), DESC_INT_CODE_ADDR, KMEM_MAP_CODE);
    }

    // TODO: set up TSS for IST
    for(int i = 0; i < 256; i ++) {
        DESC_INT_TASKS_MEM[i] = 0;
        idt_set(i, *(uint64_t *)(DESC_INT_CODE_ADDR + i*8), 0, 0);
    }

    /* load new IDT */
    {
        uint8_t idt_pointer[10];
        *(uint16_t *)idt_pointer = 0xffff;
        *(uint64_t *)(idt_pointer+2) = DESC_IDT_ADDR;
        __asm__ __volatile__("lidt [rax]" : : "a"(&idt_pointer));
    }

}
