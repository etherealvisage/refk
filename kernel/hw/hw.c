#include "clib/comm.h"
#include "clib/mem.h"
#include "clib/str.h"

#include "klib/d.h"
#include "klib/task.h"

#include "../scheduler/interface.h"

#include "acpica/platform/acenv.h"
#include "acpica/acpi.h"

#include "osl.h"
#include "pci.h"
#include "ioapic.h"
#include "apics.h"

#include "rlib/global.h"
#include "rlib/heap.h"
#include "rlib/scheduler.h"

char *clone_string(const char *orig, uint64_t length) {
    char *ret = heap_alloc(length+1);
    mem_copy(ret, orig, length);
    ret[length] = 0;
    return ret;
}

static void process_pci_device(ACPI_HANDLE object) {
    ACPI_DEVICE_INFO *info;

    if(AcpiGetObjectInfo(object, &info) != AE_OK) {
        return;
    }
    uint64_t function = info->Address & 0xffff;
    uint64_t slot = info->Address >> 16;
    // get PCI type

    d_printf("    PCI address: bus 0 slot %x function %x\n", slot, function);
    uint32_t type = pci_readconfig(0, slot, function, 0);
    d_printf("    PCI type: %x\n", type);
    uint16_t vid = type & 0xffff;
    uint16_t pid = type >> 16;

    if(vid == 0x10ec && pid == 0x8139) {
        d_printf("    is RTL8139!\n");
    }
}

static ACPI_STATUS device_callback(ACPI_HANDLE object, UINT32 nesting,
    void *context, void **ret) {

    d_printf("device %x:\n", object);
    //d_printf("    nesting: %x\n", nesting);

    ACPI_BUFFER buf;
    buf.Length = ACPI_ALLOCATE_BUFFER;

    AcpiGetName(object, ACPI_FULL_PATHNAME, &buf);
    d_printf("    full name: %s\n", buf.Pointer);
    heap_free(buf.Pointer);

    ACPI_DEVICE_INFO *info;

    if(AcpiGetObjectInfo(object, &info) != AE_OK) {
        d_printf("    no _HID!\n");
        return AE_OK;
    }
    
    if(info->Valid & ACPI_VALID_HID) {
        char *cloned = clone_string(info->HardwareId.String, info->HardwareId.Length);
        d_printf("    hardware ID: %s\n", cloned);
        heap_free(cloned);
    }
    if(info->Valid & ACPI_VALID_CID) {
        for(uint64_t i = 0; i < info->CompatibleIdList.Count; i ++) {
            char *cloned = clone_string(info->CompatibleIdList.Ids[i].String,
                info->CompatibleIdList.Ids[i].Length);
            d_printf("    compatible hardware ID: %s\n", cloned);
            heap_free(cloned);
        }
    }

    d_printf("    class code length: %x\n", info->ClassCode.Length);

    if(info->HardwareId.Length &&
        !str_ncmp(info->HardwareId.String, "PNP0A08", info->HardwareId.Length)) {

        d_printf("        found PCI root!\n");

        d_printf("Getting routing table...\n");
        ACPI_BUFFER retbuf;
        retbuf.Length = ACPI_ALLOCATE_BUFFER;

        ACPI_STATUS ret = AcpiGetIrqRoutingTable(object, &retbuf);
        if(ret == AE_OK) {
            d_printf("        Routing table size: %x\n", retbuf.Length);
            uint64_t offset = 0;
            while(offset+sizeof(ACPI_PCI_ROUTING_TABLE) <= retbuf.Length) {
                ACPI_PCI_ROUTING_TABLE *routing = (void *)((uint8_t *)retbuf.Pointer + offset);
                if(routing->Length == 0) break;
                d_printf("            route %x's %x to %x (offset %x)\n",
                    routing->Address, routing->SourceIndex, routing->Pin,
                    offset);

                offset += routing->Length;
            }
        }
        else d_printf("        Routing table get failed: %x\n", ret);
    }
    else if(info->HardwareId.Length &&
        !str_ncmp(info->HardwareId.String, "PNP0C0F", info->HardwareId.Length)) {

        /*ACPI_BUFFER retbuf;
        retbuf.Length = ACPI_ALLOCATE_BUFFER;
        ACPI_STATUS ret = AcpiEvaluateObject(object, "_CRS", 0, &retbuf);

        if(ret != AE_OK) {
            d_printf("Failed to get CRS!\n");
        }
        else {
            
        }*/
        ACPI_BUFFER retbuf;
        retbuf.Length = ACPI_ALLOCATE_BUFFER;
        AcpiGetPossibleResources(object, &retbuf);

        d_printf("    possible resources:\n");
        uint64_t offset = 0;
        uint8_t *data = retbuf.Pointer;
        while(offset < retbuf.Length) {
            ACPI_RESOURCE *resource = (void *)(data + offset);
            if(resource->Type == ACPI_RESOURCE_TYPE_END_TAG) break;

            d_printf("        resource type: %x\n", resource->Type);
            if(resource->Type == ACPI_RESOURCE_TYPE_EXTENDED_IRQ) {
                d_printf("        IRQ resource!\n");
                ACPI_RESOURCE_EXTENDED_IRQ *eirq = &resource->Data.ExtendedIrq;

                d_printf("        count: %x\n", eirq->InterruptCount);
                for(uint8_t i = 0; i < eirq->InterruptCount; i ++) {
                    d_printf("            IRQ: %x\n", eirq->Interrupts[i]);
                }
            }

            offset += resource->Length;
        }
    }

    if(info->Valid & ACPI_VALID_ADR) {
        d_printf("    address: %x\n", info->Address);
    }

    return AE_OK;
}

void _start() {
    rlib_setup(RLIB_DEFAULT_HEAP, RLIB_DEFAULT_START);
    heap_init(HEAP_DEFAULT);
    heap_set_sizer(heap_rlib_sizer, 0);

    uint64_t own_id;
    comm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    ACPI_TABLE_DESC tables[32];
    ACPI_STATUS ret = AcpiInitializeTables(tables, 32, 0);

    if(ret != AE_OK) {
        d_printf("Failed to read ACPI tables!\n");
        while(1) {}
    }

    AcpiInitializeSubsystem();
    AcpiLoadTables();
    AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
    AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);

    // tell ACPI we're using the I/O APICs
    {
        ACPI_OBJECT mode;
        mode.Type = ACPI_TYPE_INTEGER;
        mode.Integer.Value = 1; // I/O APICs
        ACPI_OBJECT_LIST list;
        list.Pointer = &mode;
        list.Count = 1;
        ACPI_STATUS ret = AcpiEvaluateObject(0, "\\_PIC", &list, 0);
        if(ret != AE_OK) d_printf("Failed to invoke \\_PIC: %x\n", ret);
        else d_printf("Successfully told ACPI to use I/O APIC mode!\n");
    }

    // initialize BSP local APIC and I/O APICs
    apics_init();

    pci_probe_all();
    d_printf("PCI devices probed\n");

    {
        void *r;
        uint64_t ret = AcpiGetDevices(0, &device_callback, 0, &r);
        d_printf("get_devices ret: %x\n", ret);
    }

    d_printf("initialized\n");
    while(1) {}
}
