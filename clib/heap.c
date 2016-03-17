#include "heap.h"

static heap_sizer_t heap_sizer;
static void *heap_sizer_context;
static void *heap_start;

void heap_init(void *start) {
    heap_start = start;
}

void heap_set_sizer(heap_sizer_t sizer, void *context) {
    heap_sizer = sizer;
    heap_sizer_context = context;
}

void *heap_resize(int64_t by) {
    return heap_sizer(heap_sizer_context, by);
}

void *heap_get_start(void) {
    return heap_start;
}

extern void *dlmalloc(uint64_t);
void *heap_alloc(uint64_t size) {
    return dlmalloc(size);
}

extern void dlfree(void *);
void heap_free(void *ptr) {
    dlfree(ptr);
}
