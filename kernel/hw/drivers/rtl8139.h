#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

void rtl8139_init(uint8_t bus, uint8_t device, uint8_t function);

#endif
