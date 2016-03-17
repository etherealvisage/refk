#include "io.h"

void io_out8(uint16_t port, uint8_t value) {
    __asm__ __volatile__("out %%al, (%%dx)" : : "a"(value), "d"(port));
}

void io_out16(uint16_t port, uint16_t value) {
    __asm__ __volatile__("out %%ax, (%%dx)" : : "a"(value), "d"(port));
}

void io_out32(uint16_t port, uint32_t value) {
    __asm__ __volatile__("out %%eax, (%%dx)" : : "a"(value), "d"(port));
}

uint8_t io_in8(uint16_t port) {
    uint8_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%al" : "=a"(ret) : "d"(port));
    return ret;
}

uint16_t io_in16(uint16_t port) {
    uint16_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%ax" : "=a"(ret) : "d"(port));
    return ret;
}

uint32_t io_in32(uint16_t port) {
    uint32_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%eax" : "=a"(ret) : "d"(port));
    return ret;
}

