#ifndef KLIB_D_H
#define KLIB_D_H

#include <stdarg.h>

#ifdef NDEBUG
#define d_putchar(c)
#define d_printf(c, ...)
#define d_vprintf(c, va) 
#else
void d_putchar(char c);
void d_printf(const char *format, ...);
void d_vprintf(const char *format, va_list va);
#endif

#endif
