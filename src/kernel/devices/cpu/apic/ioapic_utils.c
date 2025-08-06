#include "ioapic_utils.h"
#include "ioapic_defs.h"
#include "apic_defs.h"
#include "ioapic.h"

#include "../../../paging/paging.h"

extern struct ioapic_shared_t ioapic;

void ioapic_write(struct ioapic_driver_t* self, u32 base, u8 reg, u32 value)
{
    self->mmio.writel(base, IOAPIC_REGSEL, reg);
    self->mmio.writel(base, IOAPIC_REGWIN, value);
}

u32 ioapic_read(struct ioapic_driver_t* self, u32 base, u8 reg)
{
    self->mmio.writel(base, IOAPIC_REGSEL, reg);
    return self->mmio.readl(base, IOAPIC_REGWIN);
}

// TODO: Add support for non-ACPI PCs, using an MP Table
// The function below only supports ACPI PCs

void ioapic_detect(void* ptr)
{
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)ptr;
    if (p->entry_type != 1) return;
    if (ioapic.current_ioapic_count >= MAX_IOAPIC_ENTRIES) return;

    madt_record_ioapic_t* ioapic_ptr = (madt_record_ioapic_t*)p;
    
    struct ioapic_info_t* ioapic_list_ptr = &ioapic.ioapic_list[ioapic.current_ioapic_count];
    ioapic_list_ptr->id = ioapic_ptr->ioapic_id;
    ioapic_list_ptr->ioapic_base = ioapic_ptr->ioapic_addr;
    ioapic_list_ptr->gsi_base = ioapic_ptr->gsi_base;

    page_manager_map_memory(ioapic_ptr->ioapic_addr, ioapic_ptr->ioapic_addr);
    u32 reg = ioapic_read(ioapic.driver, ioapic_ptr->ioapic_addr, 0x1);

    ioapic_list_ptr->max_redirs = ((reg >> 16)&0xFF)+1;

    ioapic.current_ioapic_count++;
    ioapic.max_gsi_count += ioapic_list_ptr->max_redirs;
}

void iso_detect(void* ptr)
{
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)ptr;
    if (p->entry_type != 2) return;

    madt_record_ioapic_iso_t* iso_ptr = (madt_record_ioapic_iso_t*)p;
    if (iso_ptr->irq_source >= MAX_ISO_ENTRIES) return;

    struct iso_info_t* iso_map_ptr = &ioapic.iso_map[iso_ptr->irq_source];
    iso_map_ptr->present = true;
    iso_map_ptr->legacy_irq_num = iso_ptr->irq_source;
    iso_map_ptr->flags = iso_ptr->flags;
    iso_map_ptr->gsi_num = iso_ptr->gsi;
}

int ioapic_find_id(struct ioapic_driver_t* self, u32 gsi);
void ioapic_redirect_gsi(struct ioapic_driver_t* self, u8 gsi, u8 vector, u8 lapic_id)
{
    int ioapic_id = ioapic_find_id(self, gsi);
    if (ioapic_id < 0) return;

    struct ioapic_info_t* ioapic_list_ptr = &ioapic.ioapic_list[ioapic_id];
    struct iso_info_t* iso_map_ptr = gsi<16?&ioapic.iso_map[gsi]:NULL;

    u32 index = gsi-ioapic_list_ptr->gsi_base;
    u32 base = ioapic_list_ptr->ioapic_base;

    u32 low = vector | (0<<8); // Fixed
    if (iso_map_ptr && iso_map_ptr->present)
    {
        u16 flags = iso_map_ptr->flags;
        // If flags are active low or leveled, enable low bits either way
        if (flags & 0x2) low |= (1<<13); // Active low
        if (flags & 0x8) low |= (1<<15); // Level-triggered
    }

    u32 high = lapic_id << 24;

    ioapic_write(self, base, 0x10+index*2+1, high);
    ioapic_write(self, base, 0x10+index*2  , low);
}

void ioapic_cut_gsi(struct ioapic_driver_t* self, u8 gsi)
{
    int ioapic_id = ioapic_find_id(self, gsi);
    if (ioapic_id < 0) return;

    struct ioapic_info_t* ioapic_list_ptr = &ioapic.ioapic_list[ioapic_id];

    u32 index = gsi-ioapic_list_ptr->gsi_base;
    u32 base = ioapic_list_ptr->ioapic_base;

    u32 low = ioapic_read(self, base, 0x10+index*2);
    u32 high = ioapic_read(self, base, 0x10+index*2+1);

    low |= (1<<16); // Mask that thing

    ioapic_write(self, base, 0x10+index*2+1, high);
    ioapic_write(self, base, 0x10+index*2  , low);
}

u32 ioapic_irq_to_gsi(struct ioapic_driver_t* self, u8 irq)
{
    struct iso_info_t* iso_map_ptr = irq<16?&ioapic.iso_map[irq]:NULL;
    if (iso_map_ptr && iso_map_ptr->present)
        return iso_map_ptr->gsi_num;
    return irq;
}

int ioapic_find_id(struct ioapic_driver_t* self, u32 gsi)
{
    for (u32 i=0;i<ioapic.current_ioapic_count;i++)
    {
        struct ioapic_info_t* ioapic_list_ptr = &ioapic.ioapic_list[i];
        
        // Bounds-checking GSI numbers
        if (gsi >= ioapic_list_ptr->gsi_base &&
        gsi < ioapic_list_ptr->gsi_base+ioapic_list_ptr->max_redirs)
            return ioapic_list_ptr->id;
    }
    return -1;
}