#include "klib/d.h" // debugging
#include "klib/phy.h"
#include "klib/msr.h"

#include "lapic.h"

#define LAPIC_REG_ID 0x2
#define LAPIC_REG_EOI 0xb
#define LAPIC_REG_SPURIOUS 0xf
#define LAPIC_REG_ISR 0x10
#define LAPIC_REG_ICR_LO 0x30
#define LAPIC_REG_ICR_HI 0x31
#define LAPIC_REG_TIMER 0x32
#define LAPIC_REG_TIMER_ICR 0x38
#define LAPIC_REG_TIMER_CUR 0x39
#define LAPIC_REG_TIMER_DIV 0x3e

uint64_t lapic_base;

static void lapic_enable();

static uint32_t get_reg(uint64_t index);
static void set_reg(uint64_t index, uint32_t value);

void lapic_setup() {
    lapic_base = msr_read(MSR_APIC_BASE) & ~0xfff;

    lapic_enable();
}

uint8_t lapic_id() {
    return get_reg(LAPIC_REG_ID);
}

void lapic_enable() {
    // set bit 11 in the APIC_BASE MSR to enable the APIC
    uint32_t base = msr_read(MSR_APIC_BASE);
    base |= 1<<11;
    msr_write(MSR_APIC_BASE, base);
    
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

void lapic_send_ipi(uint8_t to, uint8_t type, uint8_t vector) {
    // delay until the last IPI is sent
    while(get_reg(LAPIC_REG_ICR_LO) & (1<<12)) ;

    set_reg(LAPIC_REG_ICR_HI, ((uint64_t)to) << 24);
    set_reg(LAPIC_REG_ICR_LO, vector | (((uint64_t)type) << 8));
    // sent!
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

void lapic_timer_setup() {
    // divisor: 128
    set_reg(LAPIC_REG_TIMER_DIV,
        (get_reg(LAPIC_REG_TIMER_DIV) & ~0xb) | 0xa);

    uint32_t lvt = get_reg(LAPIC_REG_TIMER);
    // one-shot mode
    lvt &= ~(1<<18 | 1<<17);

    // masked
    lvt |= 1<<16;

    // vector
    lvt &= ~0xff;

    set_reg(LAPIC_REG_TIMER, lvt);
}

uint64_t lapic_timer_current() {
    return get_reg(LAPIC_REG_TIMER_CUR);
}

void lapic_timer_set_initial(uint64_t value) {
    set_reg(LAPIC_REG_TIMER_ICR, value);
}

void lapic_timer_periodic(uint64_t interval, uint64_t vector) {
    uint32_t lvt = get_reg(LAPIC_REG_TIMER);
    // periodic mode
    lvt &= ~(1<<18 | 1<<17);
    lvt |= 1<<17;

    // masked
    lvt |= (1<<16);

    // vector
    lvt &= ~0xff;
    lvt |= (vector & 0xff);

    set_reg(LAPIC_REG_TIMER, lvt);

    // set up interval
    set_reg(LAPIC_REG_TIMER_ICR, interval);

    // unmask
    lvt = get_reg(LAPIC_REG_TIMER);
    lvt &= ~(1<<16);
    set_reg(LAPIC_REG_TIMER, lvt);
}
