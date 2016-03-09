#include "klib/kutil.h"
#include "klib/kcomm.h"

#include "../scheduler/interface.h"

#include "acpica/platform/acenv.h"
#include "acpica/acpi.h"

#include "osl.h"

/*
    General outline:
    - parse ACPI tables
    - set up ACPICA
    - begin answering queries
*/

kcomm_t *schedin, *schedout;

void _start(kcomm_t *sin, kcomm_t *sout) {
    __asm__("sti");
    schedin = sin;
    schedout = sout;
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

    ACPI_TABLE_DESC tables[32];
    ACPI_STATUS ret = AcpiInitializeTables(tables, 32, 0);

    d_printf("we have the tables! ret: %x\n", ret);
    for(int i = 0; i < 32; i ++) {
        if(tables[i].Address == 0) break;
        d_printf("address: %x\n", tables[i].Address);
    }

    //AcpiInitializeSubsystem();
    while(1) {}
}
