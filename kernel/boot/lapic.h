#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

void lapic_init();

void lapic_send_eoi();
int lapic_ext_triggered(uint8_t vector);
// helper function
void lapic_conditional_eoi(uint8_t vector);

#endif
