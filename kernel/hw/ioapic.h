#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>

void ioapic_init(uint64_t address, uint64_t irqbase, uint64_t irqcount);

#endif
