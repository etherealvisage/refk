#ifndef KUTIL_H
#define KUTIL_H

#include <stdint.h>
#include <stdarg.h>

// debug functions
#ifdef NDEBUG
#define d_putchar(c)
#else
void d_putchar(char c);
#endif
void d_printf(const char *format, ...);
void d_vprintf(const char *format, va_list va);

// MSR manipulation
uint64_t kmsr_read(uint64_t index);
void kmsr_write(uint64_t index, uint64_t value);

// MSR defs
#include "klib/msrs.h"

// string handling functions
uint64_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, uint64_t maxlen);

#endif
