#ifndef RLIB_SEM_H
#define RLIB_SEM_H

#include <stdint.h>

typedef uint64_t __attribute__((aligned((128)))) sem_t;

void sem_init(sem_t *sem);

void sem_inc(sem_t *sem);
void sem_dec(sem_t *sem);

#endif
