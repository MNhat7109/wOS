#include "idt.h"
#include "../x86/x86.h"
#include "../stdint.h"

#define IDT_load(_tr) _x86_idt_load((_tr))

typedef struct idt_entry_t
{
    u16 offset_lo;
    u16 segment_selector;
    u8  _reserved;
    u8  attributes;
    u16 offset_hi;
} __attribute__((packed)) IDT_Entry;

typedef struct idtr_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) IDTR;

static IDT_Entry * vector_table = (IDT_Entry*)0x10000;
static IDTR idtr = { .size=2047, .offset=0x10000};

void IDT_init_entry(u8 vector, void* offset, u16 selector, u8 attributes)
{
    IDT_Entry* entry = &vector_table[vector];

    entry->offset_lo = (u32)offset & 0xFFFF;
    entry->offset_hi = ((u32)offset >> 16) & 0xFFFF;
    entry->segment_selector = selector;
    entry->attributes = attributes;
    entry->_reserved = 0;
}

void IDT_set_entry(u8 vector)
{
    IDT_Entry* entry = &vector_table[vector];
    entry->attributes |= (1<<7);
}
void IDT_clear_entry(u8 vector)
{
    IDT_Entry* entry = &vector_table[vector];
    entry->attributes &= ~(1<<7);
}

void IDT_init()
{
    //kprintf("%lx, %u, %lx", &idtr, idtr.size, idtr.offset);
    IDT_load(&idtr);
    //__asm__ volatile ("lidt %0" : : "m"(idtr));
}