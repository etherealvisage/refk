#include "osl.h"

ACPI_STATUS AcpiOsInitialize() {
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

    return 1;
}

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress,
    ACPI_SIZE Length) {

    // TODO
    return 0;
}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length) {
    // TODO
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS *PhysicalAddress) {
    // TODO

    return 0;
}

void *AcpiOsAllocate(ACPI_SIZE Size) {
    // TODO
    return 0;
}

void AcpiOsFree(void *Memory) {
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
    UINT32                  Width){ /* TODO */ return 0; }

ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width){ /* TODO */ return 0; }


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
    // TODO
}
void AcpiOsVprintf (const char *Format, va_list va) {
    // TODO
}
