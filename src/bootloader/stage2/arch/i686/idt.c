#include "idt.h"
#include "../../stdint.h"

void __attribute__((cdecl)) i686_idt_load(void* idtr);

typedef struct idt_entry_t
{
    u16 offset_lo;
    u16 segment_selector;
    u8  _reserved;
    u8  attributes;
    u16 offset_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct idtr_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) idtr_t;

static idt_entry_t vector_table[256];
static idtr_t idtr = { .size=2047, .offset=(u32)&vector_table};

void i686_idt_init_entry(u8 vector, void* offset, u16 selector, u8 attributes)
{
    idt_entry_t* entry = &vector_table[vector];

    entry->offset_lo = (u32)offset & 0xFFFF;
    entry->offset_hi = ((u32)offset >> 16) & 0xFFFF;
    entry->segment_selector = selector;
    entry->attributes = attributes;
    entry->_reserved = 0;
}

void i686_idt_set_entry(u8 vector)
{
    idt_entry_t* entry = &vector_table[vector];
    entry->attributes |= (1<<7);
}
void i686_idt_clear_entry(u8 vector)
{
    idt_entry_t* entry = &vector_table[vector];
    entry->attributes &= ~(1<<7);
}

void i686_idt_init()
{
    i686_idt_load(&idtr);
}