#ifndef KUTIL_H
#define KUTIL_H

#include <stdint.h>

// debug functions
void d_init();
void d_putchar(char c);
void d_printf(const char *format, ...);

// port I/O functions
void koutb(uint16_t port, uint8_t value);
uint8_t kinb(uint16_t port);

// memory manipulation functions
void *memset(void *memory, uint8_t v, uint64_t count);
void *memcpy(void *dest, const void *src, uint64_t count);
void *memmove(void *dest, const void *src, uint64_t count);

#endif
