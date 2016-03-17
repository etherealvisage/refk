#ifndef KLIB_MSR_H
#define KLIB_MSR_H

#include <stdint.h>

// MSR defs
#include "msrs.h"

uint64_t msr_read(uint64_t index);
void msr_write(uint64_t index, uint64_t value);

#endif
