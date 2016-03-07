#include <stdint.h>

#include "heap.h"
#include "global.h"
#include "mman.h"

static int64_t heap_size = 0;

void *rlib_expand_heap(int64_t by) {
    // currently don't support shrinking
    if(by < 0) return (uint8_t *)rlib_heap_start() + heap_size;
    uint64_t new_size = heap_size + by;
    uint64_t prev_end = (uint64_t)rlib_heap_start() + heap_size;
    uint64_t new_end = prev_end + by;
    for(uint64_t p = prev_end; p <= new_end; p += 0x1000) {
        //rlib_anonymous(
    }

    heap_size = new_size;

    return (uint8_t *)rlib_heap_start() + heap_size + by;

}
