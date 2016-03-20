#ifndef RLIB_SYNCH_H
#define RLIB_SYNCH_H

#include <stdint.h>

void synch_wait(uint64_t *address, uint64_t expected);
void synch_wake(uint64_t *address, uint64_t count);

#endif
