#include "klib/kutil.h"
#include "klib/kcomm.h"

#include "../scheduler/interface.h"

void _start(kcomm_t *sin, kcomm_t *sout) {
    __asm__("sti");
    d_printf("sin: %x\n", sin);
    d_printf("sout: %x\n", sout);

    sched_in_packet_t in;
    in.type = SCHED_SET_NAME;
    in.set_name.task_id = 0;
    memcpy(in.set_name.name, "hardware", 9);

    kcomm_put(sin, &in, sizeof(in));

    d_printf("Set name!\n");

    in.type = SCHED_GET_NAMED;
    in.req_id = 1;
    memcpy(in.get_named.name, "hardware", 9);
    kcomm_put(sin, &in, sizeof(in));

    sched_out_packet_t out;
    uint64_t len;
    while(kcomm_get(sout, &out, &len)) {}

    d_printf("Own ID: %x\n", out.get_named.task_id);

    while(1) {}
}
