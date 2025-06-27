#include "ioapic.h"
#include "../../paging/paging.h"
#include "../../string/string.h"
#include "../../stdio.h"

#define MAX_IOAPIC_ENTRIES 256
#define MAX_ISO_ENTRIES 16

#define IOAPIC_REGSEL 0x00
#define IOAPIC_REGWIN 0x10

typedef struct
{
    u32 id;
    u32 max_redirs;
    u32 ioapic_base;
    u32 gsi_base;
} __attribute__((packed)) ioapic_info_t;

typedef struct
{
    bool present;
    u8 legacy_irq_num;
    u16 flags;
    u32 gsi_num;
} __attribute__((packed)) iso_info_t;

ioapic_info_t ioapic_list[MAX_IOAPIC_ENTRIES];
iso_info_t iso_map[MAX_ISO_ENTRIES];
u32 current_ioapic_count=0;
u32 max_gsi_count=0;

void ioapic_write(u32 base, u8 reg, u32 value)
{
    *(volatile u32*)(base+IOAPIC_REGSEL) = reg;
    *(volatile u32*)(base+IOAPIC_REGWIN) = value;
}

u32 ioapic_read(u32 base, u8 reg)
{
    *(volatile u32*)(base+IOAPIC_REGSEL) = reg;
    return *(volatile u32*)(base+IOAPIC_REGWIN);
}

int ioapic_find_id(u32 gsi)
{
    for (u32 i=0;i<current_ioapic_count;i++)
    {
        if (gsi >= ioapic_list[i].gsi_base 
            && gsi < ioapic_list[i].gsi_base+ioapic_list[i].max_redirs)
            return ioapic_list[i].id;
    }
    return -1;
}

u32 IOAPIC_irq_to_gsi(u8 irq)
{
    if (irq < 16 && iso_map[irq].present)
        return iso_map[irq].gsi_num;
    return irq; // default: GSI = IRQ
}

void IOAPIC_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id)
{
    int ioapic_id = ioapic_find_id(gsi);
    if (ioapic_id < 0) return;
    
    ioapic_info_t* io = &ioapic_list[ioapic_id];
    u32 idx = gsi-io->gsi_base;
    u32 base = io->ioapic_base;

    u32 low = vector | (0<<8); // Fixed
    if (gsi<16 && iso_map[gsi].present)
    {
        u16 flags = iso_map[gsi].flags;
        if (flags & 0x2) low |= (1<<13); // Active low
        if (flags & 0x8) low |= (1<<15); // Leveled 
    }

    u32 high = lapic_id << 24;

    ioapic_write(base, 0x10+idx*2+1, high);
    ioapic_write(base, 0x10+idx*2, low);
}

void IOAPIC_detect(madt_record_entry_hdr_t* ptr)
{
    if (ptr->entry_type != 1) return;
    if (current_ioapic_count >= MAX_IOAPIC_ENTRIES) return;
    madt_record_ioapic_t* ioapic_ptr = (madt_record_ioapic_t*)ptr;

    kprintf("entry type: %u, id:%u, base=0x%x\n", ioapic_ptr->entry_hdr.entry_type, ioapic_ptr->ioapic_id, ioapic_ptr->ioapic_addr);

    ioapic_list[current_ioapic_count].id = ioapic_ptr->ioapic_id;
    ioapic_list[current_ioapic_count].ioapic_base = ioapic_ptr->ioapic_addr;
    ioapic_list[current_ioapic_count].gsi_base = ioapic_ptr->gsi_base;
    
    page_manager_map_memory(ioapic_ptr->ioapic_addr, ioapic_ptr->ioapic_addr);
    u32 reg = ioapic_read(ioapic_list[current_ioapic_count].ioapic_base, 0x1);
    ioapic_list[current_ioapic_count].max_redirs 
    = ((ioapic_read(ioapic_list[current_ioapic_count].ioapic_base, 0x1)>>16)&0xFF)+1;
    current_ioapic_count++;
    max_gsi_count+=ioapic_list[current_ioapic_count].max_redirs;
}

void ISO_detect(madt_record_entry_hdr_t* ptr)
{
    if (ptr->entry_type != 2) return;
    madt_record_ioapic_iso_t* iso_ptr = (madt_record_ioapic_iso_t*)ptr;
    if (iso_ptr->irq_source >= MAX_ISO_ENTRIES) return;

    iso_map[iso_ptr->irq_source].present = true;
    iso_map[iso_ptr->irq_source].legacy_irq_num = iso_ptr->irq_source;
    iso_map[iso_ptr->irq_source].gsi_num = iso_ptr->gsi;
    iso_map[iso_ptr->irq_source].flags = iso_ptr->flags;
}

u32 IOAPIC_get_max_gsi()
{
    return max_gsi_count;
}

bool IOAPIC_init()
{
    memset(ioapic_list, 0, sizeof(ioapic_info_t)*MAX_IOAPIC_ENTRIES);
    memset(iso_map, 0, sizeof(iso_info_t)*MAX_ISO_ENTRIES);
    APIC_scan_hdr(IOAPIC_detect);
    if (current_ioapic_count==0)
    {
        kprintf("IOAPIC: Cannot detect any APICs!\n");
        return false;
    }
    APIC_scan_hdr(ISO_detect);

    for (u32 i=0;i<current_ioapic_count;i++)
    {
        kprintf("IOAPIC: id=%u, max redirection entries: %u, base=0x%x, GSI base=%u\n",
        ioapic_list->id, ioapic_list->max_redirs, ioapic_list->ioapic_base, ioapic_list->gsi_base);
    }

    for (u32 i=0;i<MAX_ISO_ENTRIES;i++)
    {
        kprintf("IOAPIC: iso no. %u: ", i);
        if (!iso_map[i].present)
        {
            kprintf("Absent\n");
            continue;
        }
        kprintf(" Present, GSI: %u, IRQ: %u, flags=%x\n", iso_map[i].gsi_num, iso_map[i].legacy_irq_num, iso_map[i].flags);
    }
    return true;
}