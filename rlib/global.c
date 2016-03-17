#include "clib/comm.h"

static void *g_heap_start, *g_map_start;

void rlib_setup(void *heap_start, void *map_start) {
    g_heap_start = heap_start;
    g_map_start = map_start;
}

void *rlib_heap_start() {
    return g_heap_start;
}

void *rlib_map_start() {
    return g_map_start;
}
