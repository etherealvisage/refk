#ifndef KLIB_IO_H
#define KLIB_IO_H

#include <stdint.h>

// port I/O functions
void io_out8(uint16_t port, uint8_t value);
void io_out16(uint16_t port, uint16_t value);
void io_out32(uint16_t port, uint32_t value);
uint8_t io_in8(uint16_t port);
uint16_t io_in16(uint16_t port);
uint32_t io_in32(uint16_t port);

#endif
