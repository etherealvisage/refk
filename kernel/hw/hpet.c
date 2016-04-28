#include "klib/d.h"
#include "klib/phy.h"
#include "klib/lapic.h"

#include "acpica/acpi.h"

#include "hpet.h"

#define HPET_GCIDR  0x00
#define HPET_GCR    0x02
#define HPET_MCR    0x1e
#define HPET_CCR(n) (0x20 + 4*(n))
#define HPET_CVR(n) (0x21 + 4*(n))
#define HPET_FSB(n) (0x22 + 4*(n))

static uint64_t hpet_base;

static void hpet_write(uint32_t index, uint64_t value) {
    phy_write64(hpet_base + index*8, value);
}

static uint64_t hpet_read(uint32_t index) {
    return phy_read64(hpet_base + index*8);
}

void hpet_init() {
    // get the APIC table
    ACPI_TABLE_HEADER *hpet_table_header;
    ACPI_STATUS ret = AcpiGetTable(ACPI_SIG_HPET, 1, &hpet_table_header);
    if(ret != AE_OK) {
        d_printf("Failed to find HPET table!\n");
        while(1) {}
    }
    ACPI_TABLE_HPET *hpet_table = (void *)hpet_table_header;

    hpet_base = hpet_table->Address.Address;

    d_printf("Found HPET at 0x%x\n", hpet_base);

    uint64_t gcidr = hpet_read(HPET_GCIDR);
    uint64_t counters = ((gcidr >> 8) & 0x1f) + 1;
    uint64_t between_ticks = (gcidr >> 32);

    uint64_t gcr = hpet_read(HPET_GCR);
    gcr |= 1; // set enable bit
    gcr &= ~2; // clear legacy interrupt routing
    hpet_write(HPET_GCR, gcr);

    // XXX: make this more configurable in the future

    // set up timer 0 to be a 1us periodic timer delivered by MSI on int 0x60
    uint64_t ccr = hpet_read(HPET_CCR(0));
    if((ccr & (1<<15)) == 0) {
        d_printf("No FSB/MSI signalling supported on HPET timer 0!\n");
        while(1) {}
    }

    // clear enabled bit, set to edge-triggered
    ccr &= ~7;
    // set periodic, base offset, FSB flags
    ccr |= 1<<3 | 1<<6 | 1<<14;

    hpet_write(HPET_CCR(0), ccr);

    // timer: 1 ms
    const uint64_t period = 1000000 * 1000000ul;
    uint64_t ticks = period / between_ticks;

    hpet_write(HPET_CVR(0), hpet_read(HPET_MCR) + ticks);
    hpet_write(HPET_CVR(0), ticks);

    // set up fsb delivery for current CPU
    uint64_t addr = 0xfee00000 | (lapic_id() << 12);
    uint64_t data = 0x60; // vector = 0x60, edge-triggered
    hpet_write(HPET_FSB(0), addr << 32 | data);

    ccr = hpet_read(HPET_CCR(0));
    // set enabled flag
    ccr |= 1<<2;
    hpet_write(HPET_CCR(0), ccr);
}
