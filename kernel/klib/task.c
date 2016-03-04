#include "klib/task.h"
#include "klib/elf.h"
#include "klib/kmem.h"
#include "klib/kutil.h"

#define DEFAULT_TASK_STACK_TOP 0x80000000

static task_state_t *find_available() {
    task_state_t *ts = 0;
    for(int i = 1; i < NUM_TASKS; i ++) {
        task_state_t *t = TASK_MEM(i);
        if(t->state & TASK_STATE_VALID) continue;
        ts = t;
        break;
    }

    return ts;
}

task_state_t *task_create(void *elf_image, uint64_t stack_size) {
    task_state_t *ts = find_available();
    if(!ts) return 0;

    d_printf("task_create() called\n");

    ts->cr3 = kmem_create_root();

    d_printf("    new root created\n");

    // round stack size up
    stack_size = (stack_size + 0xfff) & ~0xfff;
    uint64_t stack_bottom = DEFAULT_TASK_STACK_TOP;
    while(stack_size > 0) {
        stack_bottom -= 0x1000;
        stack_size -= 0x1000;

        uint64_t page = kmem_getpage();
        kmem_map(ts->cr3, stack_bottom, page, KMEM_MAP_DATA);
    }

    ts->cs = 0x08;
    ts->ds = 0x10;
    ts->es = 0x10;
    ts->fs = 0x10;
    ts->gs = 0x10;
    ts->ss = 0x10;
    ts->rflags = 0x2; // TODO: make this more sensible
    ts->rsp = DEFAULT_TASK_STACK_TOP;

    // map in ELF
    d_printf("    mapping in ELF\n");
    Elf64_Ehdr *header = elf_image;
    Elf64_Phdr *phdrs = (void *)((uint8_t *)elf_image + header->e_phoff);
    for(int i = 0; i < header->e_phnum; i ++) {
        if(phdrs[i].p_type != PT_LOAD) continue;

        // ensure everything has backed memory
        uint64_t start = phdrs[i].p_vaddr & ~0xfff;
        uint64_t end = phdrs[i].p_vaddr + phdrs[i].p_memsz;

        while(start < end) {
            kmem_map(ts->cr3, start, kmem_getpage(), KMEM_MAP_DATA);
            start += 0x1000;
        }

        kmem_memcpy(ts->cr3, phdrs[i].p_vaddr,
            (uint8_t *)elf_image + phdrs[i].p_offset,
            phdrs[i].p_filesz);

        // remap executable regions as KMEM_MAP_CODE
        // TODO: support regular read-only regions...
        if(phdrs[i].p_flags & PF_X || !(phdrs[i].p_flags & PF_W)) {
            start = phdrs[i].p_vaddr & ~0xfff;
            end = phdrs[i].p_vaddr + phdrs[i].p_memsz;
            while(start < end) {
                kmem_set_flags(ts->cr3, start, KMEM_MAP_CODE);
                start += 0x1000;
            }
        }
    }

    d_printf("    mapped!\n");

    ts->rip = header->e_entry;
    ts->state = TASK_STATE_VALID;

    return ts;
}

task_state_t *task_create_local(void *entry, void *stack_top) {
    task_state_t *ts = find_available();
    if(!ts) return 0;

    ts->cr3 = kmem_current();

    ts->cs = 0x08;
    ts->ds = 0x10;
    ts->es = 0x10;
    ts->fs = 0x10;
    ts->gs = 0x10;
    ts->ss = 0x10;
    ts->rflags = 0x2; // TODO: make this more sensible
    ts->rsp = (uint64_t)stack_top;
    ts->rip = (uint64_t)entry;
    ts->state = TASK_STATE_VALID;

    return ts;
}
