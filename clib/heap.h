#ifndef CLIB_HEAP_H
#define CLIB_HEAP_H

#include <stdint.h>

#define HEAP_DEFAULT ((void *)0x600000000000)

typedef void *(*heap_sizer_t)(void *context, int64_t amount);

void heap_init(void *start);
void heap_set_sizer(heap_sizer_t sizer, void *context);
void *heap_resize(int64_t by);
void *heap_get_start(void);

void *heap_alloc(uint64_t size);
void heap_free(void *ptr);

#endif
