#include "apic.h"
#include "../../cpuid/cpuid.h"
#include "../../paging/paging.h"
#include "../../stdio.h"
#include "lapic.h"
#include "ioapic.h"

u32 record_section_length=0;
madt_t* madt;

void APIC_scan_hdr(madt_callback_t callback)
{
    u8* entry = (u8*)&madt->records[0];
    u8* entry_end = (u8*)((u32)entry+record_section_length); 
    while (entry < entry_end)
    {
        u8 entry_type = ((madt_record_entry_hdr_t*)entry)->entry_type;
        u8 len = ((madt_record_entry_hdr_t*)entry)->record_length;
        // kprintf("entry: 0x%x\n", entry);
        // kprintf("entry type: %u\n", entry_type);

        if (len < 2 || entry+len>entry_end) break;
        callback(((madt_record_entry_hdr_t*)entry));
        entry+=len;
    }
}

bool APIC_init()
{
    madt = (madt_t*)ACPI_find_table("APIC");
    if (!madt)
    {
        kprintf("APIC: MADT not found!\n");
        return false;
    } 
    page_manager_map_memory((u32)madt, (u32)madt);

    u64 apic_base = rdmsr(0x1B);
    apic_base |= (1<<11);
    u32 lapic_phys = madt->lapic_addr;
    apic_base &= ~(0xFFFFFF000ULL);
    apic_base |= lapic_phys;
    wrmsr(0x1B, apic_base);

    page_manager_map_memory(lapic_phys, lapic_phys);
    if (!LAPIC_init(lapic_phys))
    {
        kprintf("APIC: Failed to init LAPIC!\n");
        return false;
    }

    LAPIC_write(0xF0, LAPIC_SVR_APIC | (0xFF<<0)); // Enable APIC + 16 Spurious Interrupts
    LAPIC_write(0x80, 0);

    record_section_length = madt->table_hdr.length-sizeof(acpi_sdt_hdr_t)-8;

    if (!IOAPIC_init())
    {
        kprintf("APIC: Failed to init IOAPIC!\n");
        return false;
    }
    return true;
}