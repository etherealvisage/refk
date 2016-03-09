// make this file quiet
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "klib/kcomm.h"
#include "klib/kutil.h"

#include "../scheduler/interface.h"

#include "osl.h"

#define MAP_BEGIN 0x70000000

uint64_t last_map = MAP_BEGIN;

uint64_t this_id;
kcomm_t *schedin, *schedout;

ACPI_STATUS AcpiOsInitialize() {
    d_printf("initializing...\n");
    __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(this_id));
    __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
    __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));
    return 0;
}

ACPI_STATUS AcpiOsTerminate() {
    return 0;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() {
    ACPI_SIZE ret;
    //ret = 0xf69e0;
	AcpiFindRootPointer(&ret);
	return ret;
}

ACPI_STATUS AcpiOsPredefinedOverride(
    const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue) {

    *NewValue = 0;

    return 0;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable,
    ACPI_TABLE_HEADER **NewTable) {
    
    *NewTable = 0;

    return 0;
}

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable,
    ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength) {

    // TODO: figure out how to make this succeed...

    return 1;
}

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress,
    ACPI_SIZE Length) {
    d_printf("mapping\n");

    uint64_t *process_task = (void *)0xffffffffffe01400;
    for(int i = 0; i < 32; i ++) {
        d_printf("process_task[%x] = %x\n", i, process_task[i]);
    }

    if(this_id == 0) {
        // NOTE: this is here in case MapMemory is called before anything else
        __asm__ __volatile__("mov %%gs:0x00, %%rax" : "=a"(this_id));
        __asm__ __volatile__("mov %%gs:0x08, %%rax" : "=a"(schedin));
        __asm__ __volatile__("mov %%gs:0x10, %%rax" : "=a"(schedout));
    }

    uint64_t begin = PhysicalAddress & ~0xfff;
    uint64_t end = (PhysicalAddress + Length + 0xfff) & ~0xfff;

    sched_in_packet_t in;
    in.type = SCHED_MAP_PHYSICAL;
    in.req_id = PhysicalAddress;
    in.map_physical.phy_addr = begin;
    in.map_physical.size = end-begin;
    in.map_physical.root_id = 0;
    in.map_physical.address = last_map;

    void *ret = (void *)last_map;

    last_map += end-begin;

    kcomm_put(schedin, &in, sizeof(in));

    d_printf("requesting that scheduler process requests for %x\n", this_id);

    __asm__ __volatile__("int $0xfe" : : "a"(this_id));

    d_printf("waiting for map...\n");

    sched_out_packet_t out;
    uint64_t out_len;
    while(kcomm_get(schedout, &out, &out_len) || out.req_id != in.req_id) {}

    d_printf("mapped!\n");

    return (uint8_t *)ret + (PhysicalAddress & 0xfff);
}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length) {
    d_printf("unmapping\n");
    uint64_t begin = (uint64_t)where & ~0xfff;
    uint64_t end = ((uint64_t)where + length + 0xfff) & ~0xfff;


    sched_in_packet_t in;
    in.type = SCHED_UNMAP;
    in.req_id = 0; // no confirmation requested
    in.unmap.root_id = 0;
    in.unmap.size = end-begin;
    in.unmap.address = begin;

    kcomm_put(schedin, &in, sizeof(in));
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS *PhysicalAddress) {
    d_printf("TODO: getphyaddr\n");
    // TODO

    return 0;
}

void *AcpiOsAllocate(ACPI_SIZE Size) {
    d_printf("TODO: allocate\n");
    // TODO
    return 0;
}

void AcpiOsFree(void *Memory) {
    d_printf("TODO: free\n");
    // TODO
}

ACPI_STATUS
AcpiOsReadPciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Reg,
    UINT64                  *Value,
    UINT32                  Width){ /* TODO */ return 0; }

ACPI_STATUS
AcpiOsWritePciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Reg,
    UINT64                  Value,
    UINT32                  Width) { /* TODO */ return 0; }



BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length) {
    // TODO
    return 0;
}

BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length) {
    // TODO
    return 0;
}

UINT64 AcpiOsGetTimer (void) {
    // TODO
    return 0;
}

ACPI_STATUS AcpiOsSignal (UINT32 Function, void *Info) {
    // TODO
    return 0;
}

ACPI_THREAD_ID AcpiOsGetThreadId() {
    // TODO
    return 0;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type,
    ACPI_OSD_EXEC_CALLBACK Function, void *Context) {
    // TODO
    return 0;
}

void AcpiOsWaitEventsComplete (void) {
    // TODO
}

void AcpiOsSleep(UINT64 Milliseconds) {
    // TODO
}

void AcpiOsStall(UINT32 Microseconds) {
    // TODO
}

ACPI_STATUS
AcpiOsReadPort (
    ACPI_IO_ADDRESS         Address,
    UINT32                  *Value,
    UINT32                  Width) { /* TODO */ return 0; }

ACPI_STATUS
AcpiOsWritePort (
    ACPI_IO_ADDRESS         Address,
    UINT32                  Value,
    UINT32                  Width) { /* TODO */ return 0; }



ACPI_STATUS
AcpiOsReadMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width){
    d_printf("TODO: readmem\n");

    /* TODO */ return 0; }

ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width){
    d_printf("TODO: writemem\n");
    /* TODO */ return 0; }


#if 0
ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX *OutHandle) {
    // TODO
}

void AcpiOsDeleteMutex(ACPI_MUTEX Handle) {
    // TODO
}

ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout) {
    // TODO
    return 0;
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle) {
    // TODO
}
#endif

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits,
    ACPI_SEMAPHORE *OutHandle) {

    // TODO
    return 0;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    // TODO
    return 0;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units,
    UINT16 Timeout) {

    // TODO
    return 0;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    // TODO
    return 0;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle) {
    // TODO
    return 0;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
    // TODO
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    // TODO
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    // TODO
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel,
    ACPI_OSD_HANDLER Handler, void *Context) {

    // TODO
    return 0;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler) {

    // TODO
    return 0;
}

void AcpiOsPrintf (const char *Format, ...) {
    va_list va;
    va_start(va, Format);

    d_vprintf(Format, va);

    va_end(va);
}

void AcpiOsVprintf (const char *Format, va_list va) {
    d_vprintf(Format, va);
}
