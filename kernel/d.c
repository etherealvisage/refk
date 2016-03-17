#include "klib/io.h"

#include "d.h"

// first serial device
#define PORT_BASE 0x3f8

#ifndef NDEBUG
void d_init() {
    // initialize serial port
    io_out8(PORT_BASE + 1, 0x00); // disable interrupts
    io_out8(PORT_BASE + 3, 0x80); // init set baud-rate divisor (38400 baud)
    io_out8(PORT_BASE + 0, 0x03); // divisor low-byte: 3
    io_out8(PORT_BASE + 1, 0x00); // divisor high-byte: 0
    io_out8(PORT_BASE + 3, 0x03); // 8 bit data, no parity bits, one stop bit
    io_out8(PORT_BASE + 2, 0xc7); // enable and clear UART FIFO
    io_out8(PORT_BASE + 4, 0x0b); // IRQs enabled again, set RTS
}
#endif
