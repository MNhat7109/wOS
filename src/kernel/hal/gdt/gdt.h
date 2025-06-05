#pragma once
#include "../../stdint.h"

#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10
#define USER_CODE_SEG   0x18
#define USER_DATA_SEG   0x20

#define GDT_ACCESS_PRESENT (1<<7)
#define GDT_ACCESS_PVL_KRNL (0b00<<5)
#define GDT_ACCESS_PVL_USER (0b11<<5)
#define GDT_ACCESS_CODE_DATA_SEG (1<<4)
#define GDT_ACCESS_SYSTEM_SEG (0<<4)

#define GDT_ACCESS_EXECUTABLE (1<<3)
#define GDT_ACCESS_DIRECTION_DOWN (1<<2)
#define GDT_ACCESS_READ_WRITE (1<<1)

#define GDT_ACCESS_TSS32_AVL 0x9
#define GDT_ACCESS_TSS32_BSY 0xB

#define GDT_FLAG_PAGE_GRAN (1<<3)
#define GDT_FLAG_SIZE      (1<<2)


typedef struct
{
    u16 limit_lo;
    u16 base_lo;
    u8  base_mid;
    u8  access;
    u8  limit_flags;
    u8  base_hi;
} __attribute__((packed)) gdt_entry_t;

extern gdt_entry_t gdt[6];

void GDT_set_attr(gdt_entry_t* gdt_entry, u32 base, u32 limit, u8 access, u8 flags);
void GDT_mark_present(gdt_entry_t* gdt_entry);
void GDT_init();