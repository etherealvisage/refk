#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

#define KMEM_MAP_DEFAULT 0x7

void kmem_init(uint64_t *regions);

void kmem_unuse(uint64_t page);
uint64_t kmem_getpage();

uint64_t kmem_boot();
void kmem_map(uint64_t root, uint64_t vaddr, uint64_t page, uint16_t flags);

#endif
