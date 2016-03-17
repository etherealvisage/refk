#include <stdarg.h>
#include <stdint.h>

#include "kutil.h"

#include "io.h"

// first serial device
#define PORT_BASE 0x3f8

#ifndef NDEBUG
void d_putchar(char c) {
    while((io_in8(PORT_BASE + 5) & 0x20) == 0) ;

    io_out8(PORT_BASE, c);
}
#endif

void d_printf(const char *msg, ...) {
    va_list va;
    va_start(va, msg);

    d_vprintf(msg, va);

    va_end(va);
}

void d_vprintf(const char *msg, va_list va) {
    const char *p = msg;
    while(*p) {
        if(*p == '\n') {
            d_putchar('\r');
            d_putchar('\n');
            p ++;
        }
        else if(*p == '%' && *(p+1)) {
            p ++;
            if(*p == 's') {
                const char *s = va_arg(va, const char *);
                while(*s) {
                    d_putchar(*s);
                    s ++;
                }
            }
            else if(*p == 'x' || *p == 'p') {
                uint64_t v = va_arg(va, uint64_t);
                for(int shift = 60; shift >= 0; shift -= 4) {
                    uint8_t vv = (v >> shift) & 0xf;
                    d_putchar("0123456789abcdef"[vv]);
                }
            }

            p ++;
        }
        else {
            d_putchar(*p);
            p ++;
        }
    }
}

uint64_t kmsr_read(uint64_t index) {
    uint32_t low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(index));
    return low | ((uint64_t)high << 32);
}

void kmsr_write(uint64_t index, uint64_t value) {
    __asm__ __volatile__("wrmsr" : : "c"(index), "a"(value & 0xffffffff),
        "d"(value >> 32));
}

uint64_t strlen(const char *s) {
    uint64_t ret = 0;
    while(*s) ret ++, s ++;
    return ret;
}

int strcmp(const char *s1, const char *s2) {
    while(1) {
        if(*s1 == 0 && *s2 == 0) return 0;
        if(*s1 < *s2) return -1;
        if(*s1 > *s2) return 1;
        s1 ++, s2 ++;
    }
}

int strncmp(const char *s1, const char *s2, uint64_t maxlen) {
    while(maxlen) {
        if(*s1 == 0 && *s2 == 0) return 0;
        if(*s1 < *s2) return -1;
        if(*s1 > *s2) return 1;
        s1 ++, s2 ++;
        maxlen --;
    }
    return 0;
}
