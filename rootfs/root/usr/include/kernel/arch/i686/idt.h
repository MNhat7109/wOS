#pragma once
#include <stdint.h>

#define IDT_ATTR_PVL_RING(_n) ((_n) << 5)

#define IDT_ATTR_GATE_INT   0b1110
#define IDT_ATTR_GATE_TRAP  0b1111
#define IDT_ATTR_GATE_TASK  0b0101

void __attribute__((cdecl)) idt_load_table();

void idt_create_entry(u8 vector, u32 offset, u16 selector, u8 attributes);
void idt_mark_present(u8 vector);
void idt_unmark_present(u8 vector);