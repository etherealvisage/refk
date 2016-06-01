#include "mem.h"
#include "klib/d.h"

#define USE_ES_INSTR

void *mem_set(void *memory, uint8_t v, uint64_t count) {
#ifdef USE_ES_INSTR
    __asm__ __volatile__("rep stosb" : : "a"(v), "di"(memory), "c"(count));
    return memory;
#else
    uint8_t *m8 = memory;
    while(count > 0) {
        *m8 = v;
        m8 ++;
        count --;
    }
    return memory;
#endif
}

void *mem_copy(void *dest, const void *src, uint64_t count) {
#ifdef USE_ES_INSTR
    if(count < 128) return mem_move(dest, src, count);

    __asm__ __volatile__("rep movsb" : : "di"(dest), "S"(src), "c"(count));
    return dest;
#else
    return mem_move(dest, src, count);
#endif
}

void *mem_move(void *dest, const void *src, uint64_t count) {
    if(dest < src) {
        const uint8_t *s8 = src;
        uint8_t *d8 = dest;
        while(count--) {
            *d8 = *s8;
            d8 ++, s8 ++;
        }
    }
    else {
        const uint8_t *s8 = (uint8_t *)src + count - 1;
        uint8_t *d8 = (uint8_t *)dest + count - 1;

        while(count--) {
            *d8 = *s8;
            d8 --, s8 --;
        }
    }

    return dest;
}
