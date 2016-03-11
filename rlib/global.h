#ifndef RLIB_GLOBAL_H
#define RLIB_GLOBAL_H

#define RLIB_DEFAULT_HEAP ((void *)0x700000000000)

void rlib_setup(void *heap_start);

void *rlib_heap_start();

#endif
