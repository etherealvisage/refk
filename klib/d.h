#ifndef KLIB_D_H
#define KLIB_D_H

#include <stdarg.h>

#ifdef NDEBUG
#define d_putchar(c) do { } while(0)
#define d_printf(c, ...) do { } while(0)
#define d_vprintf(c, va) do { } while(0)
#else
void d_putchar(char c);
void d_printf(const char *format, ...);
void d_vprintf(const char *format, va_list va);
#endif

#endif
