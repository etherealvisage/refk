#ifndef RLIB_MMAN_H
#define RLIB_MMAN_H

#include <stdint.h>

#define RLIB_ADDRESS_DONTCARE ((uint64_t)0)

typedef struct rlib_memory_space_t rlib_memory_space_t;

void rlib_current_memory_space(rlib_memory_space_t *mspace);
uint64_t rlib_anonymous(uint64_t address, uint64_t size);
void rlib_anonymous_remote(rlib_memory_space_t *mspace, uint64_t address,
    uint64_t size);
void rlib_copy(uint64_t address, rlib_memory_space_t *origin,
    uint64_t oaddress, uint64_t size);
void rlib_copy_remote(rlib_memory_space_t *mspace, uint64_t address,
    uint64_t size, rlib_memory_space_t *origin, uint64_t oaddress);

#endif
