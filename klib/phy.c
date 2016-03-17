#include "phy.h"
#include "clib/mem.h"

#define PHY_MAP_BASE (uint8_t *)0xffffc00000000000ULL


uint8_t phy_read8(uint64_t address) {
    return *(uint8_t *)(PHY_MAP_BASE + address);
}

uint16_t phy_read16(uint64_t address) {
    return *(uint16_t *)(PHY_MAP_BASE + address);
}

uint32_t phy_read32(uint64_t address) {
    return *(uint32_t *)(PHY_MAP_BASE + address);
}

uint64_t phy_read64(uint64_t address) {
    return *(uint64_t *)(PHY_MAP_BASE + address);
}

void phy_read(uint64_t address, void *buffer, uint64_t count) {
    mem_copy(buffer, PHY_MAP_BASE + address, count);
}

void phy_write8(uint64_t address, uint8_t value) {
    *(uint8_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write16(uint64_t address, uint16_t value) {
    *(uint16_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write32(uint64_t address, uint32_t value) {
    *(uint32_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write64(uint64_t address, uint64_t value) {
    *(uint64_t *)(PHY_MAP_BASE + address) = value;
}

void phy_write(uint64_t address, const void *buffer, uint64_t count) {
    mem_copy(PHY_MAP_BASE + address, buffer, count);
}
