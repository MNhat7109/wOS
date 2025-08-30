#pragma once
#include <libk/stdint.h>

typedef struct idt_entry_t
{
    u16 offset_lo;
    u16 segment_selector;
    u8  ist; // This is reserved in i386
    u8  attributes;
    u16 offset_hi;

#ifdef __x86_64__
    u32 offset64_hi;
    u32 _reserved;
#endif
} __attribute__((packed)) idt_entry_t;

#define GATE_TYPE_INTERRUPT 0b1110
#define GATE_TYPE_TRAP      0b1111
#define GATE_TYPE_TASK      0b0101

void x86_IDT_init_entry(u8 vector, void* offset, u16 selector, u8 attributes);
void x86_IDT_set_entry(u8 vector);
void x86_IDT_clear_entry(u8 vector);
void x86_IDT_set_table_address(void* idt_base_addr);
void x86_IDT_init(void* idt_base_addr);
void x86_IDT_load_entries(usize vector_count);