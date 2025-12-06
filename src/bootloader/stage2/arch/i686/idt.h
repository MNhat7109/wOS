#pragma once
#include "../../stdint.h"

enum GATE_TYPE
{
    INTERRUPT_GATE = 0b1110,
    TRAP_GATE = 0b1111,
    TASK_GATE = 0b0101
};

void i686_idt_init_entry(u8 vector, void* offset, u16 selector, u8 attributes);
void i686_idt_set_entry(u8 vector);
void i686_idt_clear_entry(u8 vector);
void i686_idt_init();
