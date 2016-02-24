#include <stdarg.h>
#include <stdint.h>

#include "kutil.h"

// first serial device
#define PORT_BASE 0x3f8

uint8_t kinb(uint16_t port) {
    uint8_t ret;
    __asm__("in %%al, (%%dx)" : "=a"(ret) : "d"(port));
    return ret;
}

void koutb(uint16_t port, uint8_t value) {
    __asm__("outb (%%dx), %%al" : : "a"(value), "d"(port));
}

void d_init() {
    // initialize serial port
    koutb(PORT_BASE + 1, 0x00); // disable interrupts
    koutb(PORT_BASE + 3, 0x80); // init set baud-rate divisor (38400 baud)
    koutb(PORT_BASE + 0, 0x03); // divisor low-byte: 3
    koutb(PORT_BASE + 1, 0x00); // divisor high-byte: 0
    koutb(PORT_BASE + 3, 0x03); // 8 bit data, no parity bits, one stop bit
    koutb(PORT_BASE + 2, 0xc7); // enable and clear UART FIFO
    koutb(PORT_BASE + 4, 0x0b); // IRQs enabled again, set RTS
}

void d_putchar(char c) {
    while((kinb(PORT_BASE + 5) & 0x20) == 0) ;

    koutb(PORT_BASE, c);
}

void d_printf(const char *msg, ...) {
    va_list va;
    va_start(va, msg);

    const char *p = msg;
    while(*p) {
        if(*p == '\n') {
            d_putchar('\n');
            d_putchar('\r');
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
                break;
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

    va_end(va);
}

void *memset(void *memory, uint8_t v, uint64_t count) {
    uint8_t *m8 = memory;
    while(count > 0) {
        *m8 = v;
        m8 ++;
        count --;
    }
    return memory;
}

void *memcpy(void *dest, const void *src, uint64_t count) {
    return memmove(dest, src, count);
}

void *memmove(void *dest, const void *src, uint64_t count) {
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
