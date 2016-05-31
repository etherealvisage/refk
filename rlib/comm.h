#ifndef RLIB_COMM_H
#define RLIB_COMM_H

#include <stdint.h>

#include "clib/comm.h"

int comm_read(comm_t *cc, void *data, uint64_t *data_size, int blocking);
int comm_write(comm_t *cc, void *data, uint64_t data_size);
void comm_flush(comm_t *cc);

#endif
