#include <stdint.h>

#include "kutil.h"
#include "kmem.h"
#include "kcomm.h"

#define MAX_TASKS 32

typedef struct task_state {
    uint64_t registers[16];
    uint64_t valid;
} task_state_t;

task_state_t tasks[MAX_TASKS];
task_state_t *current_task;

// executes on the task stack (1024 bytes)
// should set current_task to whatever the next task to use is
void select_next_task() {
    // simple round-robin selection
    const task_state_t *end = tasks + MAX_TASKS;
    while(1) {
        current_task ++;
        if(current_task == end) current_task = tasks;

        if(current_task->valid) break;
    }
}

extern void yield();
extern void enter_task();

void thread_wrapper(void (*function)()) {
    function();
    current_task->valid = 0;
    yield();
}

void make_thread(void (*function)(), void *stack_memory, uint64_t stack_size) {
    for(int i = 0; i < MAX_TASKS; i ++) {
        if(tasks[i].valid != 0) continue;

        tasks[i].registers[5] = (uint64_t)function;
        tasks[i].registers[7] = (uint64_t)stack_memory + stack_size;
        tasks[i].registers[8] = 0x2; // rflags
        tasks[i].registers[9] = (uint64_t)thread_wrapper;

        tasks[i].valid = 1;
        break;
    }
}

void idle_thread() {
    while(1) {
        yield();
    }
}

void producer_thread() {
    kcomm_t *kc = (void *)(0x12340000);
    kcomm_init(kc, 0x1000);

    uint64_t counter = 0;
    while(1) {
        kcomm_put(kc, &counter, 8);
        for(int i = 0; i < 100000000; i ++) {
            ;
        }
        yield();
        counter ++;
    }
}

void consumer_thread() {
    kcomm_t *kc = (void *)(0x23450000);
    while(1) {
        uint64_t packet;
        uint64_t packet_length;
        if(kcomm_get(kc, &packet, &packet_length) == 0) {
            d_printf("Received packet! %x\n", packet);
        }
        else d_printf("No packet...\n");
        yield();
    }
}

void init_thread() {
    d_printf("Running init thread...\n");

    //*(uint8_t *)(0xdeadc0de) = 0xff;
    while(1) { yield(); }
}

char producer_stack[4096];

void kmain(uint64_t *mem) {
    d_init(); // set up the serial port
    kmem_init(mem); // initialize bootstrap memory manager

    // clear the main screen
    for(int i = 0; i < 80*24*2; i ++) {
        phy_write8(0xb8000 + i, 0);
    }

    for(int i = 0; i < MAX_TASKS; i ++) {
        tasks[i].valid = 0;
    }

    char idle_stack[64];
    make_thread(idle_thread, idle_stack, 64);

    char init_stack[256];
    make_thread(init_thread, init_stack, 256);

    {
        uint64_t comm_page = kmem_getpage();
        kmem_map(kmem_boot(), 0x12340000, comm_page, KMEM_MAP_DEFAULT);
        kmem_map(kmem_boot(), 0x23450000, comm_page, KMEM_MAP_DEFAULT);
    }

    make_thread(producer_thread, producer_stack, 4096);
    char consumer_stack[256];
    make_thread(consumer_thread, consumer_stack, 256);

    // start with first thread
    current_task = tasks + 0;

    // switch from boot task to new task
    enter_task();

    // should never be reached!
    while(1) {}
}
