#include "msr.h"

uint64_t msr_read(uint64_t index) {
    uint32_t low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(index));
    return low | ((uint64_t)high << 32);
}

void msr_write(uint64_t index, uint64_t value) {
    __asm__ __volatile__("wrmsr" : : "c"(index), "a"(value & 0xffffffff),
        "d"(value >> 32));
}
