#include "klib/d.h"
#include "klib/lapic.h"
#include "klib/phy.h"

#include "ioapic.h"

#define IOAPIC_ADDR_REG (base_address)
#define IOAPIC_DATA_REG (base_address + 0x10)

static uint64_t base_address;

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

static uint64_t ioapic_get_linecount() {
    const uint32_t index = 0x01;
    uint32_t version = ioapic_read(index);

    // maximum index is one byte at bits 16:23
    // count is maximum index + 1
    return ((version >> 16) & 0xff) + 1;
}

void ioapic_init(uint64_t address, uint64_t irqbase) {
    uint64_t irqcount = ioapic_get_linecount();

    base_address = address;
    for(uint64_t i = 0; i < irqcount; i ++) {
        ioapic_set_irq(i, lapic_id(), 0x80 + irqbase + i);
    }
}
