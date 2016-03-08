#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

#define KMEM_BASE_ADDR 0xffffffffffa00000

#define KMEM_MAP_DEFAULT 0x7
#define KMEM_MAP_DATA (0x3 | (1ULL<<63))
#define KMEM_MAP_CODE (0x5)

#define KMEM_FLAG_MASK (0xfff | (1ULL<<63))

void kmem_setup(void);
void kmem_setup_bootstrap(uint64_t last);

void kmem_unuse(uint64_t page);
uint64_t kmem_getpage(void);

uint64_t kmem_paging_addr(uint64_t root, uint64_t address, uint8_t level,
    uint8_t *ok);
uint64_t kmem_current(void);
uint64_t kmem_create_root(void);
void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint64_t flags);
void kmem_set_flags(uint64_t root, uint64_t vaddr, uint64_t flags);

void kmem_memcpy(uint64_t root, uint64_t vaddr, void *data, uint64_t size);

#endif
