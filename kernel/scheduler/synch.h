#ifndef SYNCH_H
#define SYNCH_H

#include <stdint.h>

typedef struct synch_wait_t {
    struct synch_wait_t *next;
    uint64_t task_id;
} synch_wait_t;

typedef struct synchobj_t {
    uint64_t phy_addr;
    synch_wait_t *head;
    uint64_t mapcount;
} synchobj_t;

synchobj_t *synch_find_from_phy(uint64_t phy);

int synch_wait(synchobj_t *object, uint64_t value);
void synch_wake(synchobj_t *object, uint64_t value, uint64_t count);

#endif
