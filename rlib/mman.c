#include <stdint.h>

#include "klib/kutil.h" //  debugging

#include "../kernel/scheduler/interface.h"

#include "mman.h"
#include "heap.h"
#include "kcomm.h"
#include "sequence.h"
#include "global.h"

#include "mman_private.h"

uint64_t last_map_addr;

static uint64_t rlib_get_memory_address(uint64_t size) {
    if(last_map_addr == 0) last_map_addr = (uint64_t)rlib_map_start();

    uint64_t addr = last_map_addr;
    last_map_addr += size;
    last_map_addr += 0x1000; // buffer
    return addr;
}

void rlib_current_memory_space(rlib_memory_space_t *mspace) {
    // TODO
    mspace->root_id = 0;
}

uint64_t rlib_anonymous(uint64_t address, uint64_t size) {
    if(address == 0) {
        address = rlib_get_memory_address(size);
    }

    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    sched_in_packet_t in;
    in.type = SCHED_MAP_ANONYMOUS;
    in.req_id = rlib_sequence();
    in.map_anonymous.root_id = 0; // current root
    in.map_anonymous.address = address;
    in.map_anonymous.size = size;
    kcomm_put(schedin, &in, sizeof(in));
    __asm__ __volatile__("int $0xfe" : : "a"(own_id));

    sched_out_packet_t out;
    out.req_id = 0;
    uint64_t length = sizeof(out);
    while(kcomm_get(schedout, &out, &length) || out.req_id != in.req_id) {
        length = sizeof(out);
        __asm__ __volatile__("int $0xfe" : : "a"(own_id));
    }

    return address;
}

void rlib_copy(uint64_t address, rlib_memory_space_t *origin,
    uint64_t oaddress, uint64_t size) {

    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    sched_in_packet_t in;
    in.type = SCHED_MAP_MIRROR;
    in.req_id = rlib_sequence();
    in.map_mirror.root_id = 0; // current root
    in.map_mirror.address = address;
    in.map_mirror.oroot_id = origin->root_id;
    in.map_mirror.oaddress = oaddress;
    in.map_mirror.size = size;
    kcomm_put(schedin, &in, sizeof(in));

    sched_out_packet_t out;
    uint64_t length;
    while(kcomm_get(schedout, &out, &length) || out.req_id != in.req_id) {
        __asm__ __volatile__("int $0xfe" : : "a"(own_id));
    }
}
