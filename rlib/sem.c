#include "clib/atomic.h"

#include "sem.h"
#include "synch.h"

void sem_init(sem_t *sem) {
    *sem = 0;
}

void sem_inc(sem_t *sem) {
    uint64_t current = *sem;
    while(atomic_swapcompare(sem, current, current+1)) {
        current = *sem;
    }
    if(current == 0) {
        synch_wake(sem, 1);
    }
}

void sem_dec(sem_t *sem) {
    while(1) {
        uint64_t current = *sem;
        while(current == 0) {
            synch_wait(sem, 0);
            current = *sem;
        }
        if(!atomic_swapcompare(sem, current, current-1)) break;
    }
}
