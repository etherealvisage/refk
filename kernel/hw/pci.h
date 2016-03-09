#ifndef PCI_H
#define PCI_H

#include <stdint.h>

uint32_t pci_readconfig(uint32_t bus, uint32_t slot, uint32_t fun,
    uint32_t off);
void pci_writeconfig(uint32_t bus, uint32_t slot, uint32_t fun,
    uint32_t off, uint32_t value);

#endif
