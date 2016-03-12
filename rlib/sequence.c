#include <stdint.h>

#include "sequence.h"

uint64_t rlib_sequence() {
    uint64_t ret;
    __asm__ __volatile__(
        "mov %%gs:0x400, %%rax \n"
        "inc %%rax \n"
        "mov %%rax, %%gs:0x400 \n"
        : "=a"(ret));
    return ret;
}
