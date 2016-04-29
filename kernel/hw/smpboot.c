#include "klib/d.h"
#include "klib/lapic.h"
#include "klib/phy.h"
#include "klib/kmem.h"
#include "klib/task.h"
#include "klib/msr.h"

#include "acpica/acpi.h"

#include "smpboot.h"

const char smpboot_image[] = {
#include "../images/smpboot.h"
};

const char apsched_image[] = {
#include "../images/apsched.h"
};

static void init_ap(uint8_t id);
static void ap_entry(uint64_t id);
volatile uint64_t ap_semaphore;

void smpboot_init() {
    // copy SMP boot code
    phy_write(0x4000, smpboot_image, sizeof(smpboot_image));

    // XXX: does this leak some pages?
    kmem_map(kmem_current(), 0x4000, 0x4000, KMEM_MAP_DEFAULT);

    // use current cr3
    phy_write32(0x4020 + 4, kmem_current());
    // write entry point
    phy_write64(0x4020 + 8, (uint64_t)ap_entry);

    // init each AP
    ACPI_TABLE_HEADER *madt_table_header;
    AcpiGetTable(ACPI_SIG_MADT, 1, &madt_table_header);
    ACPI_TABLE_MADT *madt_table = (void *)madt_table_header;

    uint8_t *begin = (void *)(madt_table + 1);
    uint64_t offset = 0;
    while(offset + sizeof(*madt_table) < madt_table->Header.Length) {
        ACPI_SUBTABLE_HEADER *sheader = (void *)(begin + offset);

        if(sheader->Type == ACPI_MADT_TYPE_LOCAL_APIC) {
            ACPI_MADT_LOCAL_APIC *lapic = (void *)sheader;
            // is this processor core enabled?
            if((lapic->LapicFlags & 1) && lapic->Id != lapic_id()) {
                init_ap(lapic->Id);
            }
        }
        offset += sheader->Length;
    }
    
    d_printf("Initialized all APs\n");
}

static void init_ap(uint8_t id) {
    // ID for bootstrap code
    phy_write32(0x4020 + 0, id);
    // init semaphore
    ap_semaphore = 0;

    lapic_send_ipi(id, LAPIC_IPI_INIT, 0);
    volatile uint64_t *counter = (void *)0xffff900000000000;

    // 10 ms timer
    uint64_t target = *counter + 10 * 1000000;
    while(*counter < target) ;

    lapic_send_ipi(id, LAPIC_IPI_STARTUP, 4);

    // 1 ms timer
    target = *counter + 1 * 1000000;
    while(*counter < target) ;

    // should be initialized now
    // wait for boot
    while(ap_semaphore == 0) ;
}

static void ap_entry(uint64_t id) {
    ap_semaphore = 1;

    msr_write(MSR_TSC_AUX, id);

    // enable syscall/sysret
    msr_write(MSR_EFER, msr_read(MSR_EFER) | 1);

    // TODO: load IDT
    // TODO: set up syscall MSRs

    msr_write(MSR_LSTAR, 0xffffa00000000000);
    msr_write(MSR_STAR, ((0x18UL + 3) << 48) | (0x08UL << 32));

    __asm__ __volatile__("syscall");

    while(1) {}
}
