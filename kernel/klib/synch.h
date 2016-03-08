#ifndef KLIB_SYNCH_H
#define KLIB_SYNCH_H

typedef struct spinlock_t spinlock_t;

void synch_spinlock(spinlock_t *lock);
void synch_spinunlock(spinlock_t *lock);

#endif
