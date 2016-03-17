#include "klib/kutil.h"
#include "klib/io.h"

#include "pci.h"

#include "drivers/rtl8139.h"

static void probe_bus(uint8_t bus);

uint32_t pci_readconfig(uint32_t bus, uint32_t slot, uint32_t fun,
    uint32_t off) {

    uint32_t address;

    // enable bit
    address = (1UL<<31);
    address |= bus << 16;
    address |= slot << 11;
    address |= fun << 8;
    address |= (off & 0xfc);

    io_out32(0xcf8, address);

    return io_in32(0xcfc);
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

    io_out32(0xcf8, address);
    
    io_out32(0xcfc, value);
}

static void probe_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t class = pci_readconfig(bus, device, function, 0x08);
    uint8_t baseclass = class >> 24;
    uint8_t subclass = class >> 16;
    uint8_t prog_interface = class >> 8;
    //uint8_t revision = class;

    d_printf("Found PCI device! bus %x device %x function %x\n",
        bus, device, function);

    d_printf("    base class: %x subclass: %x prog_interface: %x\n", baseclass, subclass, prog_interface);

    // is this a PCI bridge?
    if(baseclass == 0x04 && subclass == 0x06) {
        uint32_t secondary_bus = pci_readconfig(bus, device, function, 0x18);
        secondary_bus >>= 8;
        secondary_bus &= 0xff;

        probe_bus(secondary_bus);
    }
    else {
        if(baseclass == 0x00 && subclass == 0x01) {
            d_printf("    VGA-compatible device!\n");
        }
        if(baseclass == 0x02 && subclass == 0x00) {
            d_printf("    Ethernet controller!\n");
            rtl8139_init(bus, device, function);
        }
        if(baseclass == 0x03 && subclass == 0x00 && prog_interface == 0x00) {
            d_printf("    VGA-compatible controller!\n");
        }
    }
}

static void probe_device(uint8_t bus, uint8_t device) {
    // probe function 0's ID
    uint32_t id = pci_readconfig(bus, device, 0, 0x00);
    if(id == 0xffffffff) return;

    probe_function(bus, device, 0);

    uint32_t info = pci_readconfig(bus, device, 0, 0x0c);
    uint8_t header_type = info >> 16;
    // is this a multifunction device?
    if(header_type & 0x80) {
        for(int i = 1; i < 8; i ++) {
            uint32_t fid = pci_readconfig(bus, device, i, 0x00);
            if(fid == 0xffffffff) continue;

            probe_function(bus, device, i);
        }
    }
}

static void probe_bus(uint8_t bus) {
    // probe each device
    for(uint32_t i = 0; i < 32; i ++) {
        probe_device(bus, i);
    }
}

void pci_probe_all() {
    probe_bus(0);
}
