#pragma once

enum GATE_TYPE
{
    INTERRUPT_GATE = 0b1110,
    TRAP_GATE = 0b1111,
    TASK_GATE = 0b0101
};

void IDT_init_entry(unsigned char vector, void* offset, unsigned short selector, unsigned char attributes);
void IDT_set_entry(unsigned char vector);
void IDT_clear_entry(unsigned char vector);
void IDT_init();