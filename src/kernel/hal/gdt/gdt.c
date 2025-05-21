#include "gdt.h"
#include "../../x86/x86.h"
#include "../../stdio.h"

#define GDT_ACCESS_PRESENT (1<<7)
#define GDT_ACCESS_PVL_KRNL (0b00<<5)
#define GDT_ACCESS_PVL_USER (0b11<<5)
#define GDT_ACCESS_CODE_DATA_SEG (1<<4)
#define GDT_ACCESS_SYSTEM_SEG (1<<4)

#define GDT_ACCESS_EXECUTABLE (1<<3)
#define GDT_ACCESS_DIRECTION_DOWN (1<<2)
#define GDT_ACCESS_READ_WRITE (1<<1)

#define GDT_ACCESS_TSS32_AVL 0x9
#define GDT_ACCESS_TSS32_BSY 0xB

#define GDT_FLAG_PAGE_GRAN (1<<3)
#define GDT_FLAG_SIZE      (1<<2)

#define GDT_load(_tr, _c,_d) _x86_GDT_load(&(_tr), (_c),(_d))

typedef struct
{
    u16 limit_lo;
    u16 base_lo;
    u8  base_mid;
    u8  access;
    u8  limit_flags;
    u8  base_hi;
} __attribute__((packed)) gdt_entry_t;

struct gdt_descriptor_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) gdtr;

gdt_entry_t gdt[5];

void GDT_set_attr(gdt_entry_t* gdt_entry, u32 base, u32 limit, u8 access, u8 flags)
{
    gdt_entry->limit_lo = limit&0xFFFF;
    gdt_entry->base_lo = base&0xFFFF;
    gdt_entry->base_mid=(base>>16)&0xFF;
    gdt_entry->access = access;
    gdt_entry->limit_flags = ((limit>>16)&0xF) | ((flags<<4)&0xF0);
    gdt_entry->base_hi = (base>>24)&0xFF;
}

void GDT_mark_present(gdt_entry_t* gdt_entry)
{
    gdt_entry->access |= GDT_ACCESS_PRESENT;
}

void GDT_init()
{
    // NULL descriptor
    GDT_set_attr(&gdt[0], 0, 0, 0, 0);
    GDT_set_attr(&gdt[1], 0, 0xFFFFF, 
        (GDT_ACCESS_PVL_KRNL | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READ_WRITE),
    (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE));
    GDT_set_attr(&gdt[2], 0, 0xFFFFF, 
        (GDT_ACCESS_PVL_KRNL | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_READ_WRITE),
    (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE));
    GDT_set_attr(&gdt[3], 0, 0xFFFFF, 
        (GDT_ACCESS_PVL_USER | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READ_WRITE),
    (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE));
    GDT_set_attr(&gdt[4], 0, 0xFFFFF, 
        (GDT_ACCESS_PVL_USER | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_READ_WRITE),
    (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE));    
    
    for (int i=1;i<5;i++) GDT_mark_present(&gdt[i]);

    gdtr.size = sizeof(gdt)-1;
    gdtr.offset = (u32)&gdt;

    GDT_load(gdtr, KERNEL_CODE_SEG, KERNEL_DATA_SEG);
}