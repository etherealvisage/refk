#ifndef CLIB_COMM_H
#define CLIB_COMM_H

#include <stdint.h>

#define COMM_SIMPLE         0
#define COMM_MULTI          1
#define COMM_BUCKETED       2

#define COMM_BUCKETSIZE2(n) ((n) << 8)

struct comm_t;
typedef struct comm_t comm_t;

int comm_init(comm_t *cc, uint64_t length, int flags);

#endif
