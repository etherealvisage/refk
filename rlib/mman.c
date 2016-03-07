#include <stdint.h>

#include "mman.h"

struct rlib_memory_space {
    uint64_t cr3;
};

rlib_memory_space_t *rlib_allocate_memory_space() {
    return 0;
}

void rlib_anonymous(uint64_t address, uint64_t size) {
    
}

void rlib_copy(uint64_t address, uint64_t size, rlib_memory_space_t *origin,
    uint64_t oaddress) {

}
