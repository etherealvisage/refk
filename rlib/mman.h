#ifndef RLIB_MMAN_H
#define RLIB_MMAN_H

#include <stdint.h>

typedef struct rlib_memory_space rlib_memory_space_t;

rlib_memory_space_t *rlib_allocate_memory_space();
void rlib_anonymous(uint64_t address, uint64_t size);
void rlib_anonymous_remote(rlib_memory_space_t *mspace, uint64_t address,
    uint64_t size);
void rlib_copy(uint64_t address, uint64_t size, rlib_memory_space_t *origin,
    uint64_t oaddress);
void rlib_copy_remote(rlib_memory_space_t *mspace, uint64_t address,
    uint64_t size, rlib_memory_space_t *origin, uint64_t oaddress);

#endif
