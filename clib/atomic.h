#ifndef CLIB_ATOMIC_H
#define CLIB_ATOMIC_H

#include <stdint.h>

// NOTE: all atomic operations are on qwords

void atomic_write(uint64_t *address, uint64_t value);
uint64_t atomic_read(uint64_t *address);

void atomic_inc(uint64_t *address);
void atomic_dec(uint64_t *address);

uint64_t atomic_swap(uint64_t *address, uint64_t value);

int atomic_swapcompare(uint64_t *address, uint64_t expected, uint64_t value);

#endif
