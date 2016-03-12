#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

#define STATUS_BASE (0xffff900000000000)
#define STATUS_MEM ((kernel_status_t *)STATUS_BASE)

typedef struct kernel_status_t {
    uint64_t timestamp;
} kernel_status_t;

#endif
