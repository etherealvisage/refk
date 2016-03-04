#include "klib/kutil.h"
#include "klib/lapic.h"

#include "ioapic.h"

#define IOAPIC_BASE 0xfec00000

#define IOAPIC_ADDR_REG IOAPIC_BASE
#define IOAPIC_DATA_REG (IOAPIC_BASE + 0x10)

static uint32_t ioapic_read(uint64_t index) {
    phy_write32(IOAPIC_ADDR_REG, index);
    return phy_read32(IOAPIC_DATA_REG);
}

static void ioapic_write(uint64_t index, uint32_t value) {
    phy_write32(IOAPIC_ADDR_REG, index);
    phy_write32(IOAPIC_DATA_REG, value);
}

static void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector) {
    const uint32_t low_index = 0x10 + irq*2;
    const uint32_t high_index = 0x10 + irq*2 + 1;

    uint32_t high = ioapic_read(high_index);
    // set APIC ID
    high &= ~0xff000000;
    high |= apic_id << 24;
    ioapic_write(high_index, high);

    uint32_t low = ioapic_read(low_index);

    // unmask the IRQ
    low &= ~(1<<16);

    // set to physical delivery mode
    low &= ~(1<<11);

    // set to fixed delivery mode
    low &= ~0x700;

    // set delivery vector
    low &= ~0xff;
    low |= vector;

    ioapic_write(low_index, low);
}

void ioapic_init() {
    for(int i = 0; i < 24; i ++) {
        ioapic_set_irq(i, lapic_id(), 0x80 + i);
    }
}
