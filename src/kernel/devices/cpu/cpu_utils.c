#include "cpu_utils.h"
#include "cpu_defs.h"
#include "cpu.h"

#include "../acpi/acpi.h"
#include "apic/apic_defs.h"
#include "apic/lapic_defs.h"
#include "apic/lapic.h"
#include "apic/ioapic.h"

extern struct x86_cpu_shared_t x86_generic_cpu;

bool cpu_scan_madt(void (*callback)(void*))
{
    if (!x86_generic_cpu.madt)
        return false;

    u8* entry = (u8*)&x86_generic_cpu.madt->records[0];
    u8* entry_end = (u8*)((u32)entry+x86_generic_cpu.rec_len);
    while (entry<entry_end)
    {
        madt_record_entry_hdr_t* rec_entry = (madt_record_entry_hdr_t*)entry;
        u8 entry_type = rec_entry->entry_type;
        u8 len = rec_entry->record_length;

        if (len < 2 || entry+len>entry_end) break;
        callback((void*)rec_entry);
        entry+=len;
    }

    return true;
}

bool cpu_scan_mpt(void (*callback)(void*))
{
    /* TODO: Implement MP Table Scanning */
    return true;
}

bool cpu_prepare_apic_acpi()
{
    if (!x86_generic_cpu.acpi_support) 
    return false;
    
    x86_generic_cpu.madt= (madt_t*)x86_generic_cpu.acpi_driver
    ->get_table(x86_generic_cpu.acpi_driver, "APIC");
    
    if (!x86_generic_cpu.madt) return false;

    x86_generic_cpu.lapic_layer->__lapic_base = x86_generic_cpu.madt->lapic_addr;
    x86_generic_cpu.rec_len = x86_generic_cpu.madt->table_hdr.length-sizeof(acpi_sdt_hdr_t)-8;

    return true;
}

bool cpu_prepare_apic_msr(struct cpu_driver_t* driver)
{
    if (!x86_generic_cpu.msr_support || !x86_generic_cpu.cpuid_support) return false;
    x86_generic_cpu.lapic_layer->__lapic_base = 
    driver->cpu_io_layer.rdmsr(0x1B) & 0xFFFFF000;
    x86_generic_cpu.rec_len = 0;

    return true;
}

u64 cpu_timer_get_freq(u64 elapsed_time, u64 multiplier)
{
    u32 lapic_curcnt = x86_generic_cpu.lapic_layer
    ->read(x86_generic_cpu.lapic_layer, LAPIC_REG_CURRCNT);
    u32 lapic_elapsed = 0xFFFFFFFF - lapic_curcnt;
    
    u64 lapic_freq = (lapic_elapsed*multiplier)/elapsed_time;

    return lapic_freq;
}

void cpu_timer_countdown()
{
    u8 div_mode = x86_generic_cpu.lapic_layer
    ->get_div_value(LAPIC_TIMER_DIVIDE_16);

    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_DIVCFG, div_mode);
    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_LVT, ((1<<16) | ((LAPIC_TIMER_MODE_ONE_SHOT&3)<<17)));
    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_INITCNT, -1); // This will write 0xFFFFFFFF
}

void cpu_timer_init(u32 ticks, u8 vector)
{
    u8 div_mode = x86_generic_cpu.lapic_layer
    ->get_div_value(LAPIC_TIMER_DIVIDE_16);

    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_DIVCFG, div_mode);
    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_LVT, ((vector) | ((LAPIC_TIMER_MODE_PERIODIC&3)<<17)));
    x86_generic_cpu.lapic_layer->write(x86_generic_cpu.lapic_layer, 
        LAPIC_REG_INITCNT, ticks);
}

u32 cpu_irq_to_gsi(u8 irq)
{
    return x86_generic_cpu.ioapic_layer->irq_to_gsi(x86_generic_cpu.ioapic_layer, irq);
}

void cpu_redirect_gsi(u8 irq, u8 vector, u8 id)
{
    x86_generic_cpu.ioapic_layer->redirect_gsi(x86_generic_cpu.ioapic_layer, irq, vector, id);
}

void cpu_cut_gsi(u8 irq)
{
    x86_generic_cpu.ioapic_layer->cut_gsi(x86_generic_cpu.ioapic_layer, irq);
}

u32 cpu_get_core_id()
{
    return x86_generic_cpu.lapic_layer
    ->read(x86_generic_cpu.lapic_layer, LAPIC_REG_ID)>>24&0xFF;
}

void cpu_send_eoi(u8 _reserved_for_arm64)
{
    x86_generic_cpu.lapic_layer
    ->write(x86_generic_cpu.lapic_layer, LAPIC_REG_EOI, 0);
}