#include <stdint.h>

#include "heap.h"
#include "global.h"
#include "mman.h"

static int64_t heap_size = 0;

void *heap_rlib_sizer(void __attribute__((unused)) *context, int64_t by) {
    // currently don't support shrinking
    if(by < 0) return (uint8_t *)heap_get_start() + heap_size;

    by = (by+0xfff) & ~0xfff;

    uint64_t new_size = heap_size + by;
    uint64_t prev_end = (uint64_t)heap_get_start() + heap_size;
    rlib_anonymous(prev_end, by);

    uint64_t old_size = heap_size;
    heap_size = new_size;

    return (uint8_t *)heap_get_start() + old_size;
}
