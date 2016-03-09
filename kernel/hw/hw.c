#include "klib/kutil.h"
#include "klib/kcomm.h"
#include "klib/sheap.h"

#include "../scheduler/interface.h"

#include "acpica/platform/acenv.h"
#include "acpica/acpi.h"

#include "osl.h"
#include "pci.h"

char *clone_string(const char *orig, uint64_t length) {
    char *ret = sheap_alloc(length+1);
    memcpy(ret, orig, length);
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
    //AcpiHwDerivePciId();
    //AcpiHwDerivePciId(0, 0, 0);

/*
ACPI_STATUS
AcpiHwDerivePciId (
    ACPI_PCI_ID             *PciId,
    ACPI_HANDLE             RootPciDevice,
    ACPI_HANDLE             PciRegion);
*/
}

static ACPI_STATUS device_callback(ACPI_HANDLE object, UINT32 nesting,
    void *context, void **ret) {

    //d_printf("device %x:\n", object);
    //d_printf("    nesting: %x\n", nesting);

    ACPI_BUFFER buf;
    buf.Length = ACPI_ALLOCATE_BUFFER;

    AcpiGetName(object, ACPI_FULL_PATHNAME, &buf);
    d_printf("    full name: %s\n", buf.Pointer);
    sheap_free(buf.Pointer);

    ACPI_DEVICE_INFO *info;

    if(AcpiGetObjectInfo(object, &info) != AE_OK) {
        d_printf("    no _HID!\n");
        return AE_OK;
    }
    
    if(info->Valid & ACPI_VALID_HID) {
        char *cloned = clone_string(info->HardwareId.String, info->HardwareId.Length);
        d_printf("    hardware ID: %s\n", cloned);
        sheap_free(cloned);
    }
    if(info->Valid & ACPI_VALID_CID) {
        for(uint64_t i = 0; i < info->CompatibleIdList.Count; i ++) {
            char *cloned = clone_string(info->CompatibleIdList.Ids[i].String,
                info->CompatibleIdList.Ids[i].Length);
            d_printf("    compatible hardware ID: %s\n", cloned);
            sheap_free(cloned);
        }
    }

    // check parent type
    ACPI_HANDLE parent;
    AcpiGetParent(object, &parent);
    if(AcpiGetParent(object, &parent) == AE_OK) {
        ACPI_OBJECT_TYPE parent_type;
        AcpiGetType(parent, &parent_type);
        if(parent_type == ACPI_TYPE_DEVICE) {
            ACPI_DEVICE_INFO *pinfo;
            AcpiGetObjectInfo(parent, &pinfo);
            d_printf("    parent hardware ID length: %x\n", pinfo->HardwareId.Length);
            if(pinfo->HardwareId.Length > 0) {
                char *cloned = clone_string(pinfo->HardwareId.String, pinfo->HardwareId.Length);
                d_printf("    parent hardware ID: %s\n", cloned);
                sheap_free(cloned);

                // PNP0A03: PCI root device
                if(!strcmp(pinfo->HardwareId.String, "PNP0A03")) process_pci_device(object);
            }
        }
    }

    if(info->Valid & ACPI_VALID_ADR) {
        d_printf("    address: %x\n", info->Address);
    }

    return AE_OK;
}

/*
    General outline:
    - parse ACPI tables
    - set up ACPICA
    - begin answering queries
*/

void _start() {
    uint64_t own_id;
    kcomm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    sheap_init();

    ACPI_TABLE_DESC tables[32];
    ACPI_STATUS ret = AcpiInitializeTables(tables, 32, 0);

    d_printf("we have the tables! ret: %x\n", ret);
    for(int i = 0; i < 32; i ++) {
        if(tables[i].Address == 0) break;
        d_printf("    address: %x\n", tables[i].Address);
        char name[5];
        memcpy(name, tables[i].Signature.Ascii, 4);
        name[4] = 0;
        d_printf("    name: %s\n", name);
    }

    AcpiInitializeSubsystem();
    AcpiLoadTables();
    AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
    AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);

    {
        void *r;
        uint64_t ret = AcpiGetDevices(0, &device_callback, 0, &r);
        d_printf("get_devices ret: %x\n", ret);
    }

    d_printf("initialized\n");
    while(1) {}
}
