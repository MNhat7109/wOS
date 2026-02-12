#include <kernel/arch/i686/idt.h>

#define IDT_ATTR_PRESENT (1<<7)

typedef struct idt_entry_i386_t
{
    u16 offset_low;
    u16 segment_selector;
    u8  _reserved;
    u8  attributes;
    u16 offset_hi;
} __attribute__((packed)) idt_entry_i386_t;

typedef struct idtr_t
{
    u16 size;
    u32 addr_offset;
} __attribute__((packed)) idtr_t;

idt_entry_i386_t idt[256];
idtr_t idtr;

void idt_init()
{

}

void idt_create_entry(u8 vector, u32 offset, u16 selector, u8 attributes)
{
    idt_entry_i386_t* entry = &idt[vector];
    
    entry->offset_low = (u32)offset & 0xFFFF;
    entry->segment_selector = selector;
    entry->attributes = attributes;
    entry->_reserved = 0;
    entry->offset_hi = ((u32)offset >> 16) & 0xFFFF;
}

void idt_mark_present(u8 vector)
{
    idt_entry_i386_t* entry = &idt[vector];
    entry->attributes |= IDT_ATTR_PRESENT;
}

void idt_unmark_present(u8 vector)
{
    idt_entry_i386_t* entry = &idt[vector];
    entry->attributes &= ~IDT_ATTR_PRESENT;
}