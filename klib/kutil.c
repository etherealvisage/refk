#include <stdarg.h>
#include <stdint.h>

#include "kutil.h"

#include "clib/mem.h"

#define PHY_MAP_BASE (uint8_t *)0xffffc00000000000ULL

// first serial device
#define PORT_BASE 0x3f8

void koutb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("out %%al, (%%dx)" : : "a"(value), "d"(port));
}

void koutw(uint16_t port, uint16_t value) {
    __asm__ __volatile__("out %%ax, (%%dx)" : : "a"(value), "d"(port));
}

void koutd(uint16_t port, uint32_t value) {
    __asm__ __volatile__("out %%eax, (%%dx)" : : "a"(value), "d"(port));
}

uint8_t kinb(uint16_t port) {
    uint8_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%al" : "=a"(ret) : "d"(port));
    return ret;
}

uint16_t kinw(uint16_t port) {
    uint16_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%ax" : "=a"(ret) : "d"(port));
    return ret;
}

uint32_t kind(uint16_t port) {
    uint32_t ret = 0;
    __asm__ __volatile__("in (%%dx), %%eax" : "=a"(ret) : "d"(port));
    return ret;
}

#ifndef NDEBUG
void d_putchar(char c) {
    while((kinb(PORT_BASE + 5) & 0x20) == 0) ;

    koutb(PORT_BASE, c);
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

uint8_t phy_read8(uint64_t address) {
    return *(uint8_t *)(PHY_MAP_BASE + address);
}

uint16_t phy_read16(uint64_t address) {
    return *(uint16_t *)(PHY_MAP_BASE + address);
}

uint32_t phy_read32(uint64_t address) {
    return *(uint32_t *)(PHY_MAP_BASE + address);
}

uint64_t phy_read64(uint64_t address) {
    return *(uint64_t *)(PHY_MAP_BASE + address);
}

void phy_read(uint64_t address, void *buffer, uint64_t count) {
    mem_copy(buffer, PHY_MAP_BASE + address, count);
}

void phy_write8(uint64_t address, uint8_t value) {
    *(uint8_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write16(uint64_t address, uint16_t value) {
    *(uint16_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write32(uint64_t address, uint32_t value) {
    *(uint32_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write64(uint64_t address, uint64_t value) {
    *(uint64_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write(uint64_t address, const void *buffer, uint64_t count) {
    mem_copy(PHY_MAP_BASE + address, buffer, count);
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
