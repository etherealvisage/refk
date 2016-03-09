#ifndef KLIB_SYNCH_H
#define KLIB_SYNCH_H

#include <stdint.h>

struct spinlock_t {
    uint32_t value;
    uint32_t intrflag;
};
typedef struct spinlock_t spinlock_t;

struct semaphore_t {
    uint64_t value;
    spinlock_t accesslock;
};
typedef struct semaphore_t semaphore_t;

void synch_initspin(spinlock_t *lock);
void synch_spinlock(spinlock_t *lock);
void synch_spinunlock(spinlock_t *lock);

void synch_initsemaphore(semaphore_t *semaphore, uint64_t value);
void synch_semaphoreinc(semaphore_t *semaphore, uint64_t by);
void synch_semaphoredec(semaphore_t *semaphore, uint64_t by);
int synch_semaphoretrydec(semaphore_t *semaphore, uint64_t by);

#endif
