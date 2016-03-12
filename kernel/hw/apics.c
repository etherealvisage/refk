#include "klib/kutil.h"
#include "klib/lapic.h"

#include "acpica/acpi.h"

#include "apics.h"
#include "ioapic.h"

void apics_init() {
    // perform some quick initialization
    lapic_setup();
    // get the APIC table
    ACPI_TABLE_HEADER *madt_table_header;
    ACPI_STATUS ret = AcpiGetTable(ACPI_SIG_MADT, 1, &madt_table_header);
    if(ret != AE_OK) {
        d_printf("Failed to find APIC table!\n");
        while(1) {}
    }
    ACPI_TABLE_MADT *madt_table = (void *)madt_table_header;
    if(madt_table->Flags & ACPI_MADT_PCAT_COMPAT) {
        /* Disable 8259 PIC emulation. */
        /* Set ICW1 */
        koutb(0x20, 0x11);
        koutb(0xa0, 0x11);

        /* Set ICW2 (IRQ base offsets) */
        koutb(0x21, 0xe0);
        koutb(0xa1, 0xe8);

        /* Set ICW3 */
        koutb(0x21, 4);
        koutb(0xa1, 2);

        /* Set ICW4 */
        koutb(0x21, 1);
        koutb(0xa1, 1);

        /* Set OCW1 (interrupt masks) */
        koutb(0x21, 0xff);
        koutb(0xa1, 0xff);
    }

    uint8_t *begin = (void *)(madt_table + 1);
    uint64_t offset = 0;
    while(offset + sizeof(*madt_table) < madt_table->Header.Length) {
        ACPI_SUBTABLE_HEADER *sheader = (void *)(begin + offset);

        if(sheader->Type == ACPI_MADT_TYPE_IO_APIC) {
            ACPI_MADT_IO_APIC *ioapic = (void *)sheader;
            ioapic_init(ioapic->Address, ioapic->GlobalIrqBase);
        }
        else if(sheader->Type == ACPI_MADT_TYPE_LOCAL_APIC) {
            ACPI_MADT_LOCAL_APIC *lapic = (void *)sheader;
            // is this processor core enabled?
            if((lapic->LapicFlags & 1) && lapic->Id != lapic_id()) {
                d_printf("Found AP!\n");
            }
        }
        /*else if(sheader->Type == ACPI_MADT_TYPE_INTERRUPT_OVERRIDE) {
            ACPI_MADT_INTERRUPT_OVERRIDE *iover = (void *)sheader;
            //d_printf("Found interrupt override! ISA interrupt %x should be %x\n", iover->SourceIrq, iover->GlobalIrq);
        }*/
        /*else if(sheader->Type == ACPI_MADT_TYPE_LOCAL_APIC_NMI) {
            ACPI_MADT_LOCAL_APIC_NMI *nmi = (void *)sheader;
            d_printf("Found local APIC NMI! processor %x has NMI connected to %x\n", nmi->ProcessorId, nmi->Lint);
        }*/
        offset += sheader->Length;
    }

}
