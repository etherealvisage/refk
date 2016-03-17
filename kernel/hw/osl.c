// make this file quiet
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "clib/comm.h"

#include "klib/kutil.h"
#include "klib/synch.h"
#include "klib/task.h"
#include "klib/desc.h"
#include "klib/io.h"

#include "rlib/heap.h"

#include "kernel/scheduler/interface.h"

#include "osl.h"
#include "pci.h"

#define MAP_BEGIN 0x70000000

uint64_t last_map = MAP_BEGIN;

uint64_t this_id;
comm_t *schedin, *schedout;

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

    return (uint8_t *)0xffffc00000000000 + PhysicalAddress;
}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length) {

}

ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS *PhysicalAddress) {

    d_printf("TODO: getphyaddr\n");

    return 0;
}

void *AcpiOsAllocate(ACPI_SIZE Size) {
    return malloc(Size);
}

void AcpiOsFree(void *Memory) {
    free(Memory);
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Reg,
    UINT64 *Value, UINT32 Width) {

    if(Width == 8) {
        uint32_t r = pci_readconfig(PciId->Bus, PciId->Device, PciId->Function,
            Reg);
        *Value = (uint64_t) ((r >> (8*(Reg & 3))) & 0xff);
    }
    else if(Width == 16) {
        uint32_t r = pci_readconfig(PciId->Bus, PciId->Device, PciId->Function,
            Reg);
        *Value = (uint64_t) ((r >> (16*(Reg & 1))) & 0xffff);
    }
    else if(Width == 32) {
        uint32_t r = pci_readconfig(PciId->Bus, PciId->Device, PciId->Function,
            Reg);
        *Value = (uint64_t) r;
    }

    return 0;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID *PciId, UINT32 Reg,
    UINT64 Value, UINT32 Width) {
    
    d_printf("TODO: writepciconfig\n");
    return 0;
}


BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length) {
    d_printf("TODO: osreadable\n");
    return 0;
}

BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length) {
    d_printf("TODO: oswritable\n");
    return 0;
}

UINT64 AcpiOsGetTimer(void) {
    d_printf("TODO: gettimer\n");
    return 0;
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void *Info) {
    d_printf("TODO: signal\n");
    return 0;
}

ACPI_THREAD_ID AcpiOsGetThreadId() {
    return this_id;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type,
    ACPI_OSD_EXEC_CALLBACK Function, void *Context) {

    d_printf("TODO: execute\n");
    return 0;
}

void AcpiOsWaitEventsComplete (void) {
    d_printf("TODO: wait\n");
}

void AcpiOsSleep(UINT64 Milliseconds) {
    d_printf("TODO: sleep\n");
}

void AcpiOsStall(UINT32 Microseconds) {
    d_printf("TODO: stall\n");
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32 *Value,
    UINT32 Width) {

    if(Width == 8) {
        *Value = io_in8(Address);
    }
    else if(Width == 16) {
        *Value = io_in16(Address);
    }
    else if(Width == 32) {
        *Value = io_in32(Address);
    }
    else return 1;

    return 0;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value,
    UINT32 Width) {

    if(Width == 8) {
        io_out8(Address, Value);
    }
    else if(Width == 16) {
        io_out16(Address, Value);
    }
    else if(Width == 32) {
        io_out32(Address, Value);
    }
    else return 1;

    return 0;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value,
    UINT32 Width) {

    d_printf("TODO: readmem\n");

    return 0;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value,
    UINT32 Width) {

    d_printf("TODO: writemem\n");
    return 0;
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits,
    ACPI_SEMAPHORE *OutHandle) {

    semaphore_t *semaphore = malloc(sizeof(spinlock_t));

    synch_initsemaphore(semaphore, InitialUnits);

    *OutHandle = semaphore;
    return 0;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    free(Handle);
    return 0;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units,
    UINT16 Timeout) {

    //d_printf("handle: %x\n", Handle);
    synch_semaphoredec(Handle, Units);
    // TODO: timeout
    return 0;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    synch_semaphoreinc(Handle, Units);
    return 0;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle) {
    spinlock_t *lock = malloc(sizeof(spinlock_t));
    synch_initspin(lock);
    *OutHandle = lock;
    return !lock;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
    free(Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    synch_spinlock(Handle);
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    synch_spinunlock(Handle);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel,
    ACPI_OSD_HANDLER Handler, void *Context) {

    char *stack = malloc(1024);

    task_state_t *ts = task_create();
    task_set_local(ts, Handler, stack + 1024);
    ts->rdi = (uint64_t)Context;
    DESC_INT_TASKS_MEM[0x80 + InterruptLevel] = (uint64_t)ts;

    return 0;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler) {

    d_printf("TODO: removeirq\n");
    return 0;
}

void AcpiOsPrintf(const char *Format, ...) {
    va_list va;
    va_start(va, Format);

    d_vprintf(Format, va);

    va_end(va);
}

void AcpiOsVprintf(const char *Format, va_list va) {
    d_vprintf(Format, va);
}
