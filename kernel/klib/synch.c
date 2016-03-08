#include <stdint.h>

#include "synch.h"

struct spinlock_t {
    uint32_t value;
    uint32_t intrflag;
};

void synch_spinlock(spinlock_t *lock) {
    
}

void synch_spinunlock(spinlock_t *lock) {
    
}
