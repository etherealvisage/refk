#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

// should be called once per CPU, but per-task is fine
void lapic_setup(void);

uint8_t lapic_id(void);

void lapic_send_eoi(void);
int lapic_ext_triggered(uint8_t vector);
// helper function
void lapic_conditional_eoi(uint8_t vector);

#endif
