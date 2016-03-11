#ifndef RLIB_HEAP_H
#define RLIB_HEAP_H

#include <stdint.h>

void *malloc(uint64_t size);
void free(void *ptr);

#endif
