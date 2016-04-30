#ifndef HW_APICS_H
#define HW_APICS_H

void apics_init();

// returns number of ticks per ms
uint64_t apics_synchronize();

#endif
