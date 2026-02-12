#include <kernel/arch/i686/gdt.h>
#include <kernel/mmu.h>

#define GDT_ACCESS_PRESENT (1<<7)

typedef struct gdt_entry_i386_t
{
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  limit_flags;
    u8  base_hi;
} __attribute__((packed)) gdt_entry_i386_t;

typedef struct gdt_descriptor_t
{
    u16 size;
    u32 addr_offset;
} __attribute__((packed)) gdt_descriptor_t;

gdt_entry_i386_t gdt[6];
gdt_descriptor_t gdtr;

void gdt_create_entry(u32 index, u32 base, u32 limit, u8 access, u8 flags)
{
    gdt_entry_i386_t* gdt_entry = &gdt[index];

    gdt_entry->limit_low = limit&0xFFFF;
    gdt_entry->base_low = (u32)base&0xFFFF;
    gdt_entry->base_mid=((u32)base>>16)&0xFF;
    gdt_entry->access = access;
    gdt_entry->limit_flags = ((limit>>16)&0xF) | ((flags<<4)&0xF0);
    gdt_entry->base_hi = ((u32)base>>24)&0xFF;
}

void gdt_mark_present(u32 index)
{
    gdt_entry_i386_t* gdt_entry = &gdt[index];

    gdt_entry->access |= GDT_ACCESS_PRESENT;
}