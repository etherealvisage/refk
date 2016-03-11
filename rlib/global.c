#include "kcomm.h"

static void *g_heap_start;

void rlib_setup(void *heap_start) {
    g_heap_start = heap_start;
}

void *rlib_heap_start() {
    return g_heap_start;
}
