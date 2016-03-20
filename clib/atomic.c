#include "atomic.h"

#define CACHE_LINE_SIZE 64
#define CACHE_LINE_MASK (CACHE_LINE_SIZE-1)
#define CACHE_LINE_MAX (CACHE_LINE_SIZE-7)

#define CACHE_LINE_WITHIN(a) \
    ((((uint64_t)a) & (CACHE_LINE_MASK)) >= CACHE_LINE_MAX)

void atomic_write(uint64_t *address, uint64_t value) {
    if(CACHE_LINE_WITHIN(address)) {
        *address = value;
    }
    else {
        __asm__ __volatile__(
            "lock   xchg %%rbx, 0(%%rax)"
            :
            : "a"(address), "b"(value));
    }
}

uint64_t atomic_read(uint64_t *address) {
    if(CACHE_LINE_WITHIN(address)) {
        return *address;
    }
    else {
        uint64_t value;
        __asm__ __volatile__(
            "lock   addq %%rbx, 0(%%rax)"
            : "=b"(value)
            : "a"(address), "b"(0));
        return value;
    }
}

void atomic_inc(uint64_t *address) {
    __asm__ __volatile__(
        "lock incq 0(%%rax)"
        :
        : "a"(address));
}

void atomic_dec(uint64_t *address) {
    __asm__ __volatile__(
        "lock decq 0(%%rax)"
        :
        : "a"(address));
}

uint64_t atomic_swap(uint64_t *address, uint64_t value) {
    __asm__ __volatile__(
        "lock xchgq 0(%%rax), %%rbx"
        : "=b"(value)
        : "a"(address), "b"(value));

    return value;
}

int atomic_swapcompare(uint64_t *address, uint64_t expected, uint64_t value) {
    uint64_t successful;
    __asm__ __volatile__(
        "lock cmpxchg %%rcx, 0(%%rbx) \n"
        "xor %%rax, %%rax \n"
        "cmovz %%rax, %%rdx"
        : "=b"(value), "=d"(successful)
        : "a"(expected), "b"(address), "c"(value), "d"(1));
    return successful;
}
