#include "kcomm.h"

static kcomm_t *g_sched_in, *g_sched_out;
static void *g_heap_start;

void rlib_setup(kcomm_t *sched_in, kcomm_t *sched_out, void *heap_start) {
    g_sched_in = sched_in;
    g_sched_out = sched_out;
    g_heap_start = heap_start;
}

void *rlib_heap_start() {
    return g_heap_start;
}
