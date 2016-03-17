#include "clib/comm.h"
#include "clib/heap.h"

#include "heap.h"

static void *g_map_start;

void rlib_setup(void *heap_start, void *map_start) {
    g_map_start = map_start;
    heap_init(heap_start);
    heap_set_sizer(heap_rlib_sizer, 0);
}

void *rlib_map_start() {
    return g_map_start;
}
