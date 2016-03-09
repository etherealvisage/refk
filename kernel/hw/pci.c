#include "klib/kutil.h"

#include "pci.h"

uint32_t pci_readconfig(uint32_t bus, uint32_t slot, uint32_t fun,
    uint32_t off) {

    uint32_t address;

    // enable bit
    address = (1UL<<31);
    address |= bus << 16;
    address |= slot << 11;
    address |= fun << 8;
    address |= (off & 0xfc);

    koutd(0xcf8, address);

    return kind(0xcfc);
}

void pci_writeconfig(uint32_t bus, uint32_t slot, uint32_t fun,
    uint32_t off, uint32_t value) {

    uint32_t address;

    // enable bit
    address = (1UL<<31);
    address |= bus << 16;
    address |= slot << 11;
    address |= fun << 8;
    address |= (off & 0xfc);

    koutd(0xcf8, address);
    
    koutd(0xcfc, value);
}
