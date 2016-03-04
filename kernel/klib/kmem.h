#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

#define KMEM_MAP_DEFAULT 0x7
#define KMEM_MAP_DATA (0x3 | (1ULL<<63))
#define KMEM_MAP_CODE (0x5)

#define KMEM_FLAG_MASK (0xfff | (1ULL<<63))

void kmem_unuse(uint64_t page);
uint64_t kmem_getpage();

uint64_t kmem_current();
uint64_t kmem_create_root();
void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint64_t flags);
void kmem_set_flags(uint64_t root, uint64_t vaddr, uint64_t flags);

void kmem_memcpy(uint64_t root, uint64_t vaddr, void *data, uint64_t size);

#endif
