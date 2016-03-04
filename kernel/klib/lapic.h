#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

// should only be called once
void lapic_init();
// should be called once per CPU, and calling per task is not harmful
void lapic_setup();

uint8_t lapic_id();

void lapic_send_eoi();
int lapic_ext_triggered(uint8_t vector);
// helper function
void lapic_conditional_eoi(uint8_t vector);

#endif
