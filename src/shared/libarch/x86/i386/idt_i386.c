#ifdef __i386__

#include <arch/x86/common/idt.h>
#include <arch/x86/i386/common.h>

#define IDT_ATTRIBUTE_PRESENT (1<<7)

struct idtr_t
{
    u16 size;
    u32 offset;
} __attribute__((packed)) idtr;

idt_entry_t* idt;

void x86_IDT_init_entry(u8 vector, void* offset, u16 selector, u8 attributes)
{
    // For more information on the Interrupt Descriptor Table,
    // and its structures,
    // see https://wiki.osdev.org/Interrupt_Descriptor_Table.

    idt_entry_t* idt_entry = &idt[vector];

    idt_entry->offset_lo = (u32)offset&0xFFFF;
    idt_entry->offset_hi = ((u32)offset>>16)&0xFFFF;
    idt_entry->segment_selector = selector;
    idt_entry->attributes = attributes;
    idt_entry->ist = 0;
}

void x86_IDT_set_entry(u8 vector)
{
    idt_entry_t* idt_entry = &idt[vector];

    idt_entry->attributes |= IDT_ATTRIBUTE_PRESENT;
}

void x86_IDT_clear_entry(u8 vector)
{
    idt_entry_t* idt_entry = &idt[vector];

    idt_entry->attributes &= ~IDT_ATTRIBUTE_PRESENT;
}

void x86_IDT_set_table_address(void* idt_base_addr)
{
    idt = (idt_entry_t*)idt_base_addr;
}

void x86_IDT_init(void* idt_base_addr)
{
    x86_IDT_set_table_address(idt_base_addr);
}

void x86_IDT_load_entries(usize vector_count)
{
    idtr.size = vector_count-1;
    idtr.offset = (u32)idt;
    i386_IDT_load(&idtr);
}

#endif