#ifndef KERNEL_KUTIL_H
#define KERNEL_KUTIL_H

#include "klib/kutil.h"

// debug functions
#ifdef NDEBUG
#define d_init()
#else
void d_init();
#endif

#endif
