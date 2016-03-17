#ifndef KLIB_SHEAP_H
#define KLIB_SHEAP_H

#include <stdint.h>

#include "balloc.h"

void sheap_init(void);

void *sheap_alloc(uint64_t size);
void sheap_free(void *ptr);

#endif
