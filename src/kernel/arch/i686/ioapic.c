#include <kernel/sys/acpi.h>
#include <kernel/mmio.h>
#include <kernel/arch/i686/apic.h>
#include <kernel/arch/i686/ioapic.h>
#include <stddef.h>
#include <stdbool.h>

#define REG_IOAPICVER 0x1 // IOAPIC version register
#define REG_IOREDTBL(_idx) (0x10+(_idx)*2)
#define IOAPIC_REGSEL 0x00
#define IOAPIC_REGWIN 0x10

#define MAX_IOAPIC_ENTRIES 256
#define MAX_IOAPIC_ISO_ENTRIES 16

typedef struct madt_record_ioapic_t
{
    madt_record_entry_hdr_t entry_hdr;
    u8 ioapic_id;
    u8 _reserved;
    u32 ioapic_addr;
    u32 gsi_base; // Global System Interrupt Base
} __attribute__((packed)) madt_record_ioapic_t;

typedef struct madt_record_ioapic_iso_t
{
    madt_record_entry_hdr_t entry_hdr;
    u8 bus_source;
    u8 irq_source;
    u32 gsi;
    u16 flags;
} __attribute__((packed)) madt_record_ioapic_iso_t;

typedef struct madt_ioapic_nmi_src_t
{
    madt_record_entry_hdr_t entry_hdr;
    u8 nmi_source;
    u16 flags;
    u32 gsi;
} __attribute__((packed)) madt_ioapic_nmi_src_t;

typedef struct ioapic_info_t
{
    u8 version;
    u8 interrupt_pin_count; 
    u8 ioapic_id;
    u32 ioapic_base_addr;
    u32 gsi_base_num;
} ioapic_info_t;

typedef struct ioapic_iso_info_t
{
    bool present;
    u8 irq;
    u16 flags;
    u32 gsi_base_num;
} ioapic_iso_info_t;

static struct
{
    madt_t* madt;
    usize record_length;
    ioapic_info_t ioapics[MAX_IOAPIC_ENTRIES];
    ioapic_iso_info_t iso_map[MAX_IOAPIC_ISO_ENTRIES];

    usize ioapic_count;
    usize gsi_count;
} ioapic_data;

void ioapic_write(u32 base, u8 reg, u32 value)
{
    mmio_outl((void*)base, IOAPIC_REGSEL, reg);
    mmio_outl((void*)base, IOAPIC_REGWIN, value);
}

u32 ioapic_read(u32 base, u8 reg)
{
    mmio_outl((void*)base, IOAPIC_REGSEL, reg);
    return mmio_inl((void*)base, IOAPIC_REGWIN);
}

void detect_ioapic_record(madt_record_entry_hdr_t* rec)
{
    if (!rec) return;
    if (rec->entry_type != MADT_TYPE_IOAPIC) return;

    if (ioapic_data.ioapic_count >= MAX_IOAPIC_ENTRIES) return;

    madt_record_ioapic_t* ioapic_rec = (madt_record_ioapic_t*)rec;

    int st = mmio_setup_addr((void*)ioapic_rec->ioapic_addr, 4096);
    if (st < 0)
    {
        // TODO: panic!
    }

    u32 r = ioapic_read(ioapic_rec->ioapic_addr, REG_IOAPICVER);
    struct ioapic_ver_reg_info_t {
        u8 version, _r0, max_redir_entry, _r1;
    } __attribute__((packed)) reg_info = *(struct ioapic_ver_reg_info_t*)&r;

    ioapic_data.ioapics[ioapic_data.ioapic_count++] = (ioapic_info_t){
        .ioapic_id = ioapic_rec->ioapic_id,
        .ioapic_base_addr = ioapic_rec->ioapic_addr,
        .gsi_base_num = ioapic_rec->gsi_base,
        .version = reg_info.version,
        .interrupt_pin_count = reg_info.max_redir_entry+1
    };

    ioapic_data.gsi_count+=ioapic_data.ioapics[ioapic_data.ioapic_count].interrupt_pin_count;
}

void detect_iso_record(madt_record_entry_hdr_t* rec)
{
    if (!rec) return;
    if (rec->entry_type != MADT_TYPE_IOAPIC_ISO) return;

    madt_record_ioapic_iso_t* iso_rec = (madt_record_ioapic_iso_t*)rec;

    if (iso_rec->irq_source >= MAX_IOAPIC_ISO_ENTRIES) return;

    ioapic_data.iso_map[iso_rec->irq_source] = (ioapic_iso_info_t){
        .flags = iso_rec->flags,
        .irq = iso_rec->irq_source,
        .gsi_base_num = iso_rec->gsi,
        .present = true
    };
}

int ioapic_init(apic_tbl_t* madt)
{
    int st;
    if (madt)
    {
        // ACPI

        // Scan for Type-1 MADT records
        st = apic_scramble_madt(madt, detect_ioapic_record);
        if (st < 0) goto fallback;
        // Scan for Type-2 MADT records
        st = apic_scramble_madt(madt, detect_iso_record);
        if (st < 0) goto fallback;
    }
    else
    {
fallback:
        // MPS

        // TODO: Bleugh!
    }

    return 0;
}

int find_ioapic_by_gsi(u8 gsi)
{
    for (usize i=0;i<ioapic_data.ioapic_count;i++)
    {
        u32 gsi_start =ioapic_data.ioapics[i].gsi_base_num;
        u32 pin_count =ioapic_data.ioapics[i].interrupt_pin_count;
        if (gsi >= gsi_start && gsi < gsi_start+pin_count-1)
            return ioapic_data.ioapics[i].ioapic_id;
    }
    return -1;
}

ioapic_iso_info_t* ioapic_check_iso(u8 gsi)
{
    for (usize i=0;i<MAX_IOAPIC_ISO_ENTRIES;i++)
    {
        ioapic_iso_info_t* iso_entry = &ioapic_data.iso_map[i];
        if (iso_entry->present && iso_entry->gsi_base_num == gsi)
            return iso_entry;
    }
    return NULL;
}

void ioapic_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id)
{
    int ioapic_id = find_ioapic_by_gsi(gsi);
    if (ioapic_id < 0) return;

    ioapic_info_t* ioapic_entry = &ioapic_data.ioapics[ioapic_id];
    ioapic_iso_info_t* ioapic_iso_entry = ioapic_check_iso(gsi);

    u32 base = ioapic_entry->ioapic_base_addr;
    u32 index = gsi-ioapic_entry->gsi_base_num;

    u32 low = vector | (0<<8);
    u32 high = lapic_id << 24;

    if (ioapic_iso_entry && ioapic_iso_entry->present)
    {
        if (ioapic_iso_entry->flags & 0x2) low |= (1<<13); // Active low
        if (ioapic_iso_entry->flags & 0x8) low |= (1<<15); // Level triggered
    }

    ioapic_write(base, REG_IOREDTBL(index)+1, high);
    ioapic_write(base, REG_IOREDTBL(index), low);
}

void ioapic_cut_gsi(u8 gsi)
{
    int ioapic_id = find_ioapic_by_gsi(gsi);
    if (ioapic_id < 0) return;

    ioapic_info_t* ioapic_entry = &ioapic_data.ioapics[ioapic_id];

    u32 base = ioapic_entry->ioapic_base_addr;
    u32 index = gsi-ioapic_entry->gsi_base_num;

    u32 high = ioapic_read(base, REG_IOREDTBL(index)+1);
    u32 low  = ioapic_read(base, REG_IOREDTBL(index));

    low |= (1<<16);

    ioapic_write(base, REG_IOREDTBL(index)+1, high);
    ioapic_write(base, REG_IOREDTBL(index), low);

}