#ifndef SCHEDULER_COMM_H
#define SCHEDULER_COMM_H

#include "clib/comm.h"

int comm_read(comm_t *cc, void *data, uint64_t *data_size);
int comm_write(comm_t *cc, void *data, uint64_t data_size);

#endif
