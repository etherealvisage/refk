#include <stdint.h>

#include "../kernel/scheduler/interface.h"

#include "mman.h"
#include "heap.h"
#include "kcomm.h"

struct rlib_memory_space {
    uint64_t root_id;
};

rlib_memory_space_t *rlib_allocate_memory_space() {
    //rlib_memory_space_t *ret = malloc(sizeof(*ret));


    return 0;
}

void rlib_anonymous(uint64_t address, uint64_t size) {
    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));


    sched_in_packet_t in;
    in.type = SCHED_MAP_ANONYMOUS;
    in.req_id = (uint64_t)address;
    in.map_anonymous.root_id = 0; // current root
    in.map_anonymous.address = address;
    in.map_anonymous.size = size;
    kcomm_put(schedin, &in, sizeof(in));

    sched_out_packet_t out;
    uint64_t length;
    while(kcomm_get(schedout, &out, &length) || out.req_id != in.req_id) {
        __asm__ __volatile__("int $0xfe" : : "a"(own_id));
    }
}

void rlib_copy(uint64_t address, uint64_t size, rlib_memory_space_t *origin,
    uint64_t oaddress) {

}
