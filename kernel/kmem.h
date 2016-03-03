#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

#define KMEM_MAP_DEFAULT 0x7
#define KMEM_MAP_DATA (0x5 | (1ULL<<63))
#define KMEM_MAP_CODE (0x5)

void kmem_init(uint64_t *regions);

void kmem_unuse(uint64_t page);
uint64_t kmem_getpage();

uint64_t kmem_boot();
uint64_t kmem_current();
uint64_t kmem_create_root();
void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint64_t flags);

#endif
