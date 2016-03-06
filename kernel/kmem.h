#ifndef KERNEL_KMEM_H
#define KERNEL_KMEM_H

#include "klib/kmem.h"

void kmem_init(uint64_t *regions);

uint64_t kmem_boot(void);

#endif
