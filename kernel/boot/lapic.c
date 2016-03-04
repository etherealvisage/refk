#include "klib/kutil.h"

#include "lapic.h"

#define LAPIC_REG_EOI 0xb
#define LAPIC_REG_SPURIOUS 0xf
#define LAPIC_REG_ISR 0x10
#define LAPIC_REG_TIMER 0x32
#define LAPIC_REG_TIMER_ICR 0x38
#define LAPIC_REG_TIMER_DIVIDE 0x3e

uint64_t lapic_base;

static void disable_pic();
static void lapic_enable();

static uint32_t get_reg(uint64_t index);
static void set_reg(uint64_t index, uint32_t value);

void lapic_init() {
    disable_pic();
    lapic_base = kmsr_read(MSR_APIC_BASE) & ~0xfff;

    lapic_enable();

    // TODO: set up timer...
}

void lapic_enable() {
    // set bit 11 in the APIC_BASE MSR to enable the APIC
    uint32_t base = kmsr_read(MSR_APIC_BASE);
    base |= 1<<11;
    kmsr_write(MSR_APIC_BASE, base);
    
    // set the software-enable bit (8) in the spurious vector
    set_reg(LAPIC_REG_SPURIOUS,
        get_reg(LAPIC_REG_SPURIOUS) | (1<<8));
}

static uint32_t get_reg(uint64_t index) {
    return phy_read32(lapic_base + index * 0x10);
}

static void set_reg(uint64_t index, uint32_t value) {
    phy_write32(lapic_base + index * 0x10, value);
}

static void disable_pic() {
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

void lapic_send_eoi() {
    set_reg(LAPIC_REG_EOI, 0);
}

int lapic_ext_triggered(uint8_t vector) {
    uint8_t which = vector/32;
    uint32_t isr = get_reg(LAPIC_REG_ISR + which);
    uint8_t index = vector%32;
    return isr & (1<<index) ? 1 : 0;
}

void lapic_conditional_eoi(uint8_t vector) {
    if(lapic_ext_triggered(vector)) lapic_send_eoi();
}
