#pragma once
#include <stdint.h>
#include <stddef.h>
#include <kernel/sys/acpi.h>
#include <errno.h>

#define MADT_TYPE_IOAPIC 1 // MADT IOAPIC record
#define MADT_TYPE_IOAPIC_ISO 2 // MADT IOAPIC Interrupt Source Override for IRQ->GSI conversion

typedef struct madt_record_entry_hdr_t
{
    u8 entry_type;
    u8 record_length;
} __attribute__((packed)) madt_record_entry_hdr_t;

typedef struct madt_t
{
    acpi_sdt_hdr_t table_hdr;
    u32 lapic_addr;
    u32 flags;
    madt_record_entry_hdr_t records[1];
} __attribute__((packed)) madt_t;

typedef struct apic_tbl_t
{
    madt_t* madt;
    usize total_record_len;
} apic_tbl_t;

inline int apic_set_up_madt(apic_tbl_t* cont)
{
    if (!cont) return -EINVAL;

    acpi_sdt_hdr_t* madt = acpi_get_table("APIC");

    if (!madt) return -ENOTSUP;

    cont->madt = madt;
    cont->total_record_len = madt->length - offsetof(madt_t, records);
    return 0;
}

inline int apic_scramble_madt(apic_tbl_t* cont, void (*apic_madt_cb)(madt_record_entry_hdr_t*))
{
    if (!cont || !cont->madt || cont->total_record_len == 0 || !apic_madt_cb) return -EINVAL;

    u8* entry = (u8*)cont->madt->records;
    u8* entry_end = (u8*)(entry+cont->total_record_len);

    while (entry<entry_end)
    {
        madt_record_entry_hdr_t* madt_rec_entry = (madt_record_entry_hdr_t*)entry;
        if (madt_rec_entry->record_length < 2 || entry+madt_rec_entry->record_length > entry_end) break;

        apic_madt_cb(madt_rec_entry);
        entry+=madt_rec_entry->record_length;
    }
    return 0;
}