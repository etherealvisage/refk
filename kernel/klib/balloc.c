/*
    Block allocator implementation, source file.

    This implementation was written by Kent "ethereal" Williams-King and is
    hereby released into the public domain. Do what you wish with it.

    No guarantees as to the correctness of the implementation are provided.
*/

#define BALLOC_SOURCE
#include "balloc.h"

static BALLOC_NAME(allocator_t) *BALLOC_NAME(default_allocator);

size_t BALLOC_NAME(calculate_size)(int count) {
    return sizeof(BALLOC_NAME(allocator_t))
        + sizeof(BALLOC_NAME(region_t))*count;
}

int BALLOC_NAME(setup_allocator)(BALLOC_NAME(allocator_t) *allocator,
    size_t size) {

    size_t count;

    if(size < sizeof(*allocator)) return -1;

    count = size - sizeof(*allocator);
    count /= sizeof(BALLOC_NAME(region_t));

    allocator->region_count = 0;
    allocator->region_alloc_count = count;
    allocator->regions = (void *)(allocator + 1);

    return count;
}

int BALLOC_NAME(add_region)(BALLOC_NAME(allocator_t) *allocator,
    void *data, size_t data_size, size_t element_size) {

    int index;

    if(allocator->region_alloc_count == allocator->region_count) return -1;

    index = allocator->region_count++;

    allocator->regions[index].data = data;
    allocator->regions[index].element_size = element_size;
    allocator->regions[index].first = BALLOC_NAME(offset_sentinel);
    allocator->regions[index].uninit_begin = 0;
    allocator->regions[index].uninit_end = data_size;

    return index;
}

void BALLOC_NAME(set_default_allocator)(BALLOC_NAME(allocator_t) *allocator) {
    BALLOC_NAME(default_allocator) = allocator;
}

int BALLOC_NAME(get_last_error)() {
    return BALLOC_NAME(get_last_error_with)(BALLOC_NAME(default_allocator));
}

int BALLOC_NAME(get_last_error_with)(BALLOC_NAME(allocator_t) *allocator) {
    int last = allocator->error;
    allocator->error = BALLOC_UNAME(NO_ERROR);
    return last;
}

void *BALLOC_NAME(allocate)(size_t size) {
    return BALLOC_NAME(allocate_with)(BALLOC_NAME(default_allocator), size);
}

void BALLOC_NAME(free)(void *ptr) {
    BALLOC_NAME(free_with)(BALLOC_NAME(default_allocator), ptr);
}

/* TODO: add cascade support: if current region has no free memory, then
    use the next-best option */
void *BALLOC_NAME(allocate_with)(BALLOC_NAME(allocator_t) *allocator,
    size_t size) {

    BALLOC_NAME(region_t) *region;

    int i;
    int best = -1;
    for(i = 1; i < allocator->region_count; i ++) {
        if(allocator->regions[i].element_size < size) continue;
        if(best >= 0 &&
            allocator->regions[i].element_size
                >= allocator->regions[best].element_size) continue;
        best = i;
    }

    if(best == -1) {
        allocator->error = BALLOC_UNAME(INVALID_SIZE);
        return NULL;
    }

    region = allocator->regions + best;

    if(region->first != BALLOC_NAME(offset_sentinel)) {
        BALLOC_NAME(offset_t) now;

        now = region->first;
        region->first = *(BALLOC_NAME(offset_t) *)((char *)region->data + now);

        return (char *)region->data + now;
    }
    else {
        if(region->uninit_begin >= region->uninit_end ||
            region->uninit_end - region->uninit_begin < size) {
            allocator->error = BALLOC_UNAME(NO_FREE_MEMORY);
            return NULL;
        }
        else {
            void *ret = (char *)region->data + region->uninit_begin;
            region->uninit_begin += region->element_size;
            return ret;
        }
    }
}

void BALLOC_NAME(free_with)(BALLOC_NAME(allocator_t) *allocator, void *ptr) {
    int i;
    for(i = 0; i < allocator->region_count; i ++) {
        BALLOC_NAME(region_t) *region = allocator->regions + i;
        BALLOC_NAME(offset_t) offset;
        if(ptr < region->data) continue;
        if((char *)ptr > (char *)region->data + region->uninit_end) continue;

        offset = (char *)ptr - (char *)region->data;
        if(offset % region->element_size != 0) {
            allocator->error = BALLOC_UNAME(INVALID_FREE);
            return;
        }
        *(BALLOC_NAME(offset_t) *)ptr = region->first;
        region->first = offset;
        break;
    }

    if(i == allocator->region_count)
        allocator->error = BALLOC_UNAME(POINTER_NOT_IN_REGION);
}
