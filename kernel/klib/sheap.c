#include "sheap.h"

//static klib_balloc_allocator_t allocator;
static uint8_t allocator_memory[256];
static uint8_t heap_memory[4][1<<20];

void sheap_init() {
    klib_balloc_setup_allocator((void *)allocator_memory,
        sizeof(allocator_memory));

    klib_balloc_add_region((void *)allocator_memory,
        heap_memory[0], 1<<20, 8);
    klib_balloc_add_region((void *)allocator_memory,
        heap_memory[1], 1<<20, 32);
    klib_balloc_add_region((void *)allocator_memory,
        heap_memory[2], 1<<20, 128);
    klib_balloc_add_region((void *)allocator_memory,
        heap_memory[3], 1<<20, 1024);
}

void *sheap_alloc(uint64_t size) {
    return klib_balloc_allocate_with((void *)allocator_memory,
        size);
}

void sheap_free(void *ptr) {
    return klib_balloc_free_with((void *)allocator_memory,
        ptr);
}
