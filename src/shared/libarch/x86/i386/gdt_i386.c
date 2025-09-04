#ifdef __i386__
#include <arch/x86/common/gdt.h>
#include <arch/x86/i386/common.h>

#include <libk/stdio.h>

struct gdt_descriptor_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) gdtr;

gdt_entry_t* gdt;

void x86_GDT_set_table_address(void* address)
{
    gdt = (gdt_entry_t*)address;
}

void x86_GDT_set_attr(u32 idx, void* base, u32 limit, u8 access, u8 flags)
{
    gdt_entry_t* gdt_entry = &gdt[idx];

    gdt_entry->limit_lo = limit&0xFFFF;
    gdt_entry->base_lo = (u32)base&0xFFFF;
    gdt_entry->base_mid=((u32)base>>16)&0xFF;
    gdt_entry->access = access;
    gdt_entry->limit_flags = ((limit>>16)&0xF) | ((flags<<4)&0xF0);
    gdt_entry->base_hi = ((u32)base>>24)&0xFF;
}

void x86_GDT_mark_present(u32 idx)
{
    gdt_entry_t* gdt_entry = &gdt[idx];

    gdt_entry->access |= GDT_ACCESS_PRESENT;
}

void x86_GDT_init(void* gdt_base_addr)
{
    x86_GDT_set_table_address(gdt_base_addr);
    
    // In the OSDev Wiki for Global Descriptor Table
    // (See https://wiki.osdev.org/Global_Descriptor_Table)
    // The first entry of the GDT should always be a NULL descriptor
    // That is, every field in that entry should be all zeroes
    // The NULL descriptor should NOT be marked present.

    x86_GDT_set_attr(0, 0, 0, 0, 0);

    // After that, it's up to the client to set up the rest
}

void x86_GDT_load_entries(usize table_size, u16 code, u16 data)
{
    gdtr.size = table_size-1;
    gdtr.offset = (u32)gdt;

    i386_GDT_load(&gdtr, code, data);
}

#endif