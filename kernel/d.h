#ifndef KERNEL_D_H
#define KERNEL_D_H

#include "klib/d.h"

#ifdef NDEBUG
#define d_init()
#else
void d_init();
#endif

#endif
