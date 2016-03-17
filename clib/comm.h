#ifndef CLIB_COMM_H
#define CLIB_KCOMM_H

#include <stdint.h>

struct comm_t;
typedef struct comm_t comm_t;

void comm_init(comm_t *cc, uint64_t length);

int comm_put(comm_t *cc, void *data, uint64_t data_size);
int comm_get(comm_t *cc, void *data, uint64_t *data_size);

#endif
