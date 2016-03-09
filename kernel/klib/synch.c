#include <stdint.h>

#include "synch.h"
#include "sheap.h"
#include "kutil.h" // debugging


void synch_initspin(spinlock_t *lock) {
    lock->value = 0;
    lock->intrflag = 0;
}

void synch_spinlock(spinlock_t *lock) {
    __asm__ __volatile__(
        "pushfq \n"
        "pop    %%rax \n"
        "and    $0x200, %%rax \n"
        "mov    %%eax, 4(%%rdx) \n"
        "cli \n"
        "lock_tryagain: \n"
        "xor        %%rax, %%rax \n"
        "mov        $1, %%rbx \n"
        "lock cmpxchgl %%ebx, 0(%%rdx) \n"
        "jnz        lock_tryagain \n"
        :
        : "d"(lock), "a"(0), "b"(0)
    );
}

void synch_spinunlock(spinlock_t *lock) {
    __asm__ __volatile__(
        "xor    %%rax, %%rax \n"
        "pushfq \n"
        "pop    %%rax \n"
        "or     %%eax, 4(%%rdx) \n"
        "push   %%rax \n"
        "popfq \n"
        "movl   $0, 0(%%rdx)"
        :
        : "d"(lock), "a"(0), "b"(0)
    );
}

void synch_initsemaphore(semaphore_t *semaphore, uint64_t value) {
    semaphore->value = value;
    synch_initspin(&semaphore->accesslock);
}

void synch_semaphoreinc(semaphore_t *semaphore, uint64_t by) {
    synch_spinlock(&semaphore->accesslock);
    semaphore->value += by;
    synch_spinunlock(&semaphore->accesslock);
}

void synch_semaphoredec(semaphore_t *semaphore, uint64_t by) {
    while(1) {
        if(synch_semaphoretrydec(semaphore, by)) break;
        // pass off control onto next thread
        __asm__ __volatile__("int $0xff");
    }
}

int synch_semaphoretrydec(semaphore_t *semaphore, uint64_t by) {
    synch_spinlock(&semaphore->accesslock);
    if(semaphore->value >= by) {
        semaphore->value -= by;
        synch_spinunlock(&semaphore->accesslock);
        return 0;
    }
    synch_spinunlock(&semaphore->accesslock);

    return 1;
}
