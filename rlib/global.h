#ifndef RLIB_GLOBAL_H
#define RLIB_GLOBAL_H

#include "kcomm.h"

void rlib_setup(kcomm_t *in, kcomm_t *out, void *heap_start);

void *rlib_heap_start();

#endif
