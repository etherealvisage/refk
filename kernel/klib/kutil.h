#ifndef KUTIL_H
#define KUTIL_H

#include <stdint.h>
#include <stdarg.h>

// debug functions
void d_init();
void d_putchar(char c);
void d_printf(const char *format, ...);
void d_vprintf(const char *format, va_list va);

// port I/O functions
void koutb(uint16_t port, uint8_t value);
uint8_t kinb(uint16_t port);

// physical memory manipulation
uint8_t phy_read8(uint64_t address);
uint16_t phy_read16(uint64_t address);
uint32_t phy_read32(uint64_t address);
uint64_t phy_read64(uint64_t address);
void phy_read(uint64_t address, void *buffer, uint64_t count);
void phy_write8(uint64_t address, uint8_t value);
void phy_write16(uint64_t address, uint16_t value);
void phy_write32(uint64_t address, uint32_t value);
void phy_write64(uint64_t address, uint64_t value);
void phy_write(uint64_t address, const void *buffer, uint64_t count);

// MSR manipulation
uint64_t kmsr_read(uint64_t index);
void kmsr_write(uint64_t index, uint64_t value);

// MSR defs
#include "klib/msrs.h"

// memory manipulation functions
void *memset(void *memory, uint8_t v, uint64_t count);
void *memcpy(void *dest, const void *src, uint64_t count);
void *memmove(void *dest, const void *src, uint64_t count);

// string handling functions
uint64_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);

#endif
