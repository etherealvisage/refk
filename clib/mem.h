#ifndef CLIB_MEM_H
#define CLIB_MEM_H

#include <stdint.h>

// memory manipulation functions
void *mem_set(void *memory, uint8_t v, uint64_t count);
void *mem_copy(void *dest, const void *src, uint64_t count);
void *mem_move(void *dest, const void *src, uint64_t count);

#endif
