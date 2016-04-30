#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

#define LAPIC_IPI_FIXED     0
#define LAPIC_IPI_LOWEST    1
#define LAPIC_IPI_INIT      5
#define LAPIC_IPI_STARTUP   6

// should be called once per CPU, but per-task is fine
void lapic_setup(void);

uint8_t lapic_id(void);

void lapic_send_ipi(uint8_t to, uint8_t type, uint8_t vector);
void lapic_send_eoi(void);
int lapic_ext_triggered(uint8_t vector);
// helper function
void lapic_conditional_eoi(uint8_t vector);

void lapic_timer_setup();
uint64_t lapic_timer_current();
void lapic_timer_set_initial(uint64_t value);
void lapic_timer_periodic(uint64_t interval, uint64_t vector);

#endif
