/*
    Block allocator implementation, header file.

    This implementation was written by Kent "ethereal" Williams-King and is
    hereby released into the public domain. Do what you wish with it.

    No guarantees as to the correctness of the implementation are provided.
*/

#ifndef BALLOC_H
#define BALLOC_H

#include <stddef.h>

#define BALLOC_NAME(name) klib_balloc_ ## name
#define BALLOC_UNAME(name) KLIB_BALLOC_ ## name

typedef unsigned int BALLOC_NAME(offset_t);

#ifdef BALLOC_SOURCE
const BALLOC_NAME(offset_t) BALLOC_NAME(offset_sentinel) = -1;
#else
extern const BALLOC_NAME(offset_t) BALLOC_NAME(offset_sentinel);
#endif

typedef enum BALLOC_NAME(error_t) {
    BALLOC_UNAME(NO_ERROR),
    BALLOC_UNAME(INVALID_SIZE),
    BALLOC_UNAME(NO_FREE_MEMORY),
    BALLOC_UNAME(POINTER_NOT_IN_REGION),
    BALLOC_UNAME(INVALID_FREE),
    BALLOC_UNAME(UNKNOWN_ERROR)
} BALLOC_NAME(error_t);

typedef struct BALLOC_NAME(region_t) {
    void *data;
    size_t element_size;
    BALLOC_NAME(offset_t) first;
    /* NOTE: possible for uninit_begin to be > uninit_end */
    BALLOC_NAME(offset_t) uninit_begin, uninit_end;
} BALLOC_NAME(region_t);

typedef struct BALLOC_NAME(allocator_t) {
    int error;
    int region_count;
    int region_alloc_count;
    BALLOC_NAME(region_t) *regions;
} BALLOC_NAME(allocator_t);

size_t BALLOC_NAME(calculate_size)(int count);
int BALLOC_NAME(setup_allocator)(BALLOC_NAME(allocator_t) *allocator,
    size_t size);

int BALLOC_NAME(add_region)(BALLOC_NAME(allocator_t) *allocator,
    void *data, size_t data_size, size_t element_size);

void BALLOC_NAME(set_default_allocator)(BALLOC_NAME(allocator_t) *allocator);
int BALLOC_NAME(get_last_error)();
int BALLOC_NAME(get_last_error_with)(BALLOC_NAME(allocator_t) *allocator);

void *BALLOC_NAME(allocate)(size_t size);
void BALLOC_NAME(free)(void *ptr);

void *BALLOC_NAME(allocate_with)(BALLOC_NAME(allocator_t) *allocator,
    size_t size);
void BALLOC_NAME(free_with)(BALLOC_NAME(allocator_t) *allocator, void *ptr);

#endif
