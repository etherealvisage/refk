#include "clib/comm.h"
#include "clib/mem.h"
#include "clib/str.h"

#include "klib/d.h"
#include "klib/task.h"
#include "klib/msr.h"

#include "../scheduler/interface.h"

#include "acpica/platform/acenv.h"
#include "acpica/acpi.h"

#include "osl.h"
#include "pci.h"
#include "ioapic.h"
#include "apics.h"
#include "hpet.h"
#include "smpboot.h"

#include "rlib/global.h"
#include "rlib/heap.h"
#include "rlib/scheduler.h"
#include "rlib/comm.h"

#include "klib/phy.h"

static void aptest(void *offset) {
    uint8_t val = 0x41 + (uint64_t)offset;
    __asm__("sti");
    while(1) {
        uint64_t id;
        __asm__("rdtscp" : "=c"(id) : : "rax", "rdx");
        phy_write8(0xb8000 + id*2, val);
        phy_write8(0xb8001 + id*2, 0x6);
        /*phy_write8(0xb8000, val);
        phy_write8(0xb8001, 0x6);*/
    }
}

uint64_t rdtsc() {
    uint32_t low, high;
    __asm__("rdtscp" : "=a"(low), "=d"(high) : : "rcx");
    return low | ((uint64_t)high << 32);
}

#include "../scheduler/interface.h"

static void print_string(const char *string, int x, int y) {
    uint64_t d = 0xb8000 + (80*y + x)*2;
    while(*string) {
        phy_write8(d, *string);
        phy_write8(d+1, 0x6);
        d += 2;
        string ++;
    }
}

static void print_value(uint64_t value, int x, int y) {
    char string[32];
    char *p = string;
    str_lto(string, value);
    uint64_t d = 0xb8000 + (80*y + x)*2;
    while(*p) {
        phy_write8(d, *p);
        phy_write8(d+1, 0x6);
        d += 2;
        p++;
    }
}

struct sched_in_packet_t target_array[65536];

// test #1: time for batch processing 1,000,000 requests
static void aptime1(void *offset) {
    if(offset != 0) while(1) {}
    d_printf("aptime1()\n");
    uint64_t own_id;
    comm_t *schedin, *schedout;
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(own_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));

    d_printf("schedin: %x\n", schedin);
    d_printf("schedout: %x\n", schedout);

    volatile uint64_t *counter = (void *)0xffff900000000000;

    const uint64_t num = 1000000;
    uint64_t gen = 0;
    uint64_t min = -1ul;
    uint64_t max = 0;
    while(1) {
    // delay one second
    {
        uint64_t target = *counter + 1000000000;
        while(*counter < target) {}
    }

    uint64_t before = rdtsc();
    uint64_t stalls = 0;

    /*
    for(uint64_t i = 0; i < num; i ++) {
        sched_in_packet_t in;
        in.type = SCHED_PING;
        in.req_id = i+1;
        int ret = comm_write(schedin, &in, sizeof(in));
        if(ret) {
            stalls ++;
            while(comm_write(schedin, &in, sizeof(in))) { __asm__("pause"); }
        }
    }
    */

    print_string("beginning...", 6, 6);

    uint64_t target = *counter + 1000000000;
    uint64_t count = 0;
    for(; ; count ++) {
        if(*counter >= target) break;
        /*
        sched_in_packet_t in;
        in.type = SCHED_PING;
        in.req_id = count+1;
        int ret = comm_write(schedin, &in, sizeof(in));
        if(ret) {
            stalls ++;
            while(1) {
                //for(int i = 0; i < 10000000; i ++) __asm__ __volatile__("pause");
                if(!comm_write(schedin, &in, sizeof(in))) break;
            }
        }
        */
        sched_in_packet_t in;
        in.type = SCHED_PING;
        in.req_id = count+1;
        mem_copy(target_array + (count % 65536), &in, sizeof(in));
    }

    uint64_t after = rdtsc();

    print_string("duration:", 0, 0);
    print_value(after - before, 16, 0);
    print_value((after - before + num - 1) / num, 32, 0);

    if(after - before < min) min = after - before;
    print_string("min:", 0, 1);
    print_value(min, 16, 1);

    if(after - before > max) max = after - before;
    print_string("max:", 0, 2);
    print_value(max, 16, 2);

    print_string("stalls:", 0, 3);
    print_value(stalls, 16, 3);

    print_string("count:", 0, 4);
    print_value(count, 16, 4);

    print_string("generation:", 0, 5);
    print_value(gen, 16, 5);
    gen ++;
    }

    while(1) { }
        //phy_write8(0xb8000, 0x42);
        //phy_write8(0xb8001, 0x6);
    //}
}

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
    #if 0
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
    #endif

    // initialize BSP local APIC and I/O APICs
    apics_init();

    volatile uint64_t *counter = (void *)0xffff900000000000;
    volatile uint64_t *increment = (void *)0xffff900000000c00;
    *counter = 0;
    *increment = 1000000; // 1ms per tick = 1e6 ns per tick

    hpet_init();

    // begin timer
    __asm__("sti");

    uint64_t apic_ratio = apics_synchronize();

    // create simple AP tasks
    for(uint64_t i = 0; i < 3; i ++) {
        rlib_task_t task;
        rlib_create_task(RLIB_NEW_MEMSPACE, &task);
        //rlib_set_local_task(&task, aptest, (void *)i, 0x10000);
        rlib_set_local_task(&task, aptime1, (void *)i, 0x10000);
        rlib_ready_ap_task(&task);
    }

    d_printf("calling smpboot\n");
    smpboot_init(apic_ratio);

    // make default memory type writeback
    //msr_write(MSR_MTRR_DEF_TYPE, 0x0 | (1<<11));
    // mark all memory except for 0xffffc00000000000 - 0xffffcfffffffffff (phy map) as writeback
    

    rlib_reap_self();
    // mark self as not runnable...
    //TASK_MEM(2)->state &= ~TASK_STATE_RUNNABLE;

    //d_printf("task state: %x\n", TASK_MEM(2)->state);

    // give up timeslot
    __asm__("int $0xff");

    while(1) {}
}
