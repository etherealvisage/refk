#include "clib/heap.h"

#include "kmem.h"

uint64_t heap_size;

void *heap_klib_sizer(void __attribute__((unused)) *context, int64_t amount) {
    if(amount < 0) return (uint8_t *)heap_get_start() + heap_size;

    amount = (amount+0xfff) & ~0xfff;

    uint64_t new_size = heap_size + amount;
    uint64_t prev_end = (uint64_t)heap_get_start() + heap_size;
    for(uint64_t p = prev_end; p < prev_end + new_size; p += 0x1000) {
        kmem_map(kmem_current(), p, kmem_getpage(), KMEM_MAP_DATA);
    }

    uint64_t old_size = heap_size;
    heap_size = new_size;

    return (uint8_t *)heap_get_start() + old_size;
    
}
