#ifndef KLIB_HEAP_H
#define KLIB_HEAP_H

#include <stdint.h>

#include "clib/heap.h"

void *heap_klib_sizer(void *context, int64_t amount);

#endif
