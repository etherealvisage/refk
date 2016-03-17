#ifndef RLIB_GLOBAL_H
#define RLIB_GLOBAL_H

#define RLIB_DEFAULT_HEAP ((void *)0x600000000000)
#define RLIB_DEFAULT_START ((void *)0x700000000000)

void rlib_setup(void *heap_start, void *map_start);

void *rlib_map_start();

#endif
