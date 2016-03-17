#ifndef KLIB_PHY_H
#define KLIB_PHY_H

#include <stdint.h>

uint8_t phy_read8(uint64_t address);
uint16_t phy_read16(uint64_t address);
uint32_t phy_read32(uint64_t address);
uint64_t phy_read64(uint64_t address);
void phy_read(uint64_t address, void *buffer, uint64_t count);
void phy_write8(uint64_t address, uint8_t value);
void phy_write16(uint64_t address, uint16_t value);
void phy_write32(uint64_t address, uint32_t value);
void phy_write64(uint64_t address, uint64_t value);
void phy_write(uint64_t address, const void *buffer, uint64_t count);

#endif
