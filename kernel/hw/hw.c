#include "klib/kutil.h"
#include "klib/kcomm.h"
#include "klib/sheap.h"

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

void _start() {
    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    sheap_init();

    ACPI_TABLE_DESC tables[32];
    ACPI_STATUS ret = AcpiInitializeTables(tables, 32, 0);

    d_printf("we have the tables! ret: %x\n", ret);
    for(int i = 0; i < 32; i ++) {
        if(tables[i].Address == 0) break;
        d_printf("    address: %x\n", tables[i].Address);
        char name[5];
        memcpy(name, tables[i].Signature.Ascii, 4);
        name[4] = 0;
        d_printf("    name: %s\n", name);
    }

    AcpiInitializeSubsystem();

    AcpiEnableSubsystem(0);

    AcpiEnterSleepStatePrep(5);
    __asm__("cli");
    AcpiEnterSleepState(5);

    d_printf("initialized\n");
    while(1) {}
}
