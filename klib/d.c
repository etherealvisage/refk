#include "d.h"
#include "io.h"

// first serial device
#define PORT_BASE 0x3f8

#ifndef NDEBUG
void d_putchar(char c) {
    while((io_in8(PORT_BASE + 5) & 0x20) == 0) ;

    io_out8(PORT_BASE, c);
}

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
#endif
