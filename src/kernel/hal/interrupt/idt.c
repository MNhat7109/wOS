#include "idt.h"
#include "../../x86/x86.h"

#define IDT_load(_tr) _x86_IDT_load(&(_tr))

typedef struct idt_entry_t
{
    u16 offset_lo;
    u16 segment_selector;
    u8  _reserved;
    u8  attributes;
    u16 offset_hi;
} __attribute__((packed)) idt_entry_t;

idt_entry_t vector_table[256];

struct idtr_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) idtr;

void IDT_init_entry(u8 vector, void* offset, u16 selector, u8 attributes)
{
    idt_entry_t* entry = &vector_table[vector];
    entry->offset_lo = (u32)offset&0xFFFF;
    entry->offset_hi = ((u32)offset>>16)&0xFFFF;
    entry->segment_selector = selector;
    entry->attributes = attributes;
    entry->_reserved = 0;
}

void IDT_set_entry(u8 vector)
{
    idt_entry_t* entry = &vector_table[vector];
    entry->attributes |= (1<<7);
}

void IDT_clear_entry(u8 vector)
{
    idt_entry_t* entry = &vector_table[vector];
    entry->attributes &= ~(1<<7);
}

void IDT_init()
{
    idtr.size = sizeof(vector_table)-1;
    idtr.offset = (u32)vector_table;
    IDT_load(idtr);
}