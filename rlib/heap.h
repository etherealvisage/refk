#ifndef RLIB_HEAP_H
#define RLIB_HEAP_H

#include <stdint.h>

#include "clib/heap.h"

void *heap_rlib_sizer(void __attribute__((unused)) *context, int64_t by);

#endif
