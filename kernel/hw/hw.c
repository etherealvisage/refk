#include "klib/kutil.h"
#include "klib/kcomm.h"

#include "../scheduler/interface.h"

#include "acpica/platform/acenv.h"
#include "acpica/acpi.h"

/*
    General outline:
    - parse ACPI tables
    - set up ACPICA
    - begin answering queries
*/

void _start(kcomm_t *sin, kcomm_t *sout) {
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
    AcpiInitializeTables(tables, 32, 0);

    d_printf("we have the tables!\n");
    for(int i = 0; i < 32; i ++) {
        d_printf("address: %x\n", tables[0].Address);
    }

    //AcpiInitializeSubsystem();

    while(1) {}
}
