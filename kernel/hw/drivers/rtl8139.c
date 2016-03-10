#include "klib/kutil.h"

#include "../acpica/acpi.h"

#include "rtl8139.h"

void rtl8139_init(uint8_t bus, uint8_t device, uint8_t function) {
    d_printf("RTL8139 driver initializing...\n");

    d_printf("Getting IRQ routing information\n");

    //ACPI_HANDLE handle;
    //AcpiGetIrqRoutingTable(handle, 0);
}
