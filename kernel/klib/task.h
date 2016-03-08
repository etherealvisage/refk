#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define TASK_BASE 0xffffffffffe00000

#define NUM_TASKS 4096
#define TASK_ADDR(i) (TASK_BASE + 0x1000 + (i)*256)
#define TASK_MEM(i) ((task_state_t *)(TASK_ADDR(i)))

#define TASK_STATE_VALID    0x01
#define TASK_STATE_RUNNABLE 0x02

typedef struct task_state_t {
    uint64_t rax;           // 0
    uint64_t rbx;           // 1
    uint64_t rcx;           // 2
    uint64_t rdx;           // 3
    uint64_t rsi;           // 4
    uint64_t rdi;           // 5
    uint64_t rsp;           // 6
    uint64_t rbp;           // 7
    uint64_t r8;            // 8
    uint64_t r9;            // 9
    uint64_t r10;           // 10
    uint64_t r11;           // 11
    uint64_t r12;           // 12
    uint64_t r13;           // 13
    uint64_t r14;           // 14
    uint64_t r15;           // 15
    uint64_t rflags;        // 16
    uint64_t rip;           // 17
    uint64_t cs;            // 18
    uint64_t ds;            // 19
    uint64_t es;            // 20
    uint64_t fs;            // 21
    uint64_t gs;            // 22
    uint64_t ss;            // 23
    uint64_t fs_base;       // 24
    uint64_t gs_base;       // 25
    uint64_t cr3;           // 26
    uint64_t state;         // 27
    uint64_t pad1;          // 28
    uint64_t pad2;          // 29
    uint64_t pad3;          // 30
    uint64_t pad4;          // 31
} task_state_t;

task_state_t *task_create(void);

void task_load_elf(task_state_t *ts, const void *elf_image,
    uint64_t stack_size);
void task_set_local(task_state_t *ts, void *entry, void *stack_top);

void task_mark_runnable(task_state_t *ts);

#endif
