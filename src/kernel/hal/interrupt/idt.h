#pragma once
#include "../../stdint.h"

#define GATE_TYPE_INTERRUPT 0b1110
#define GATE_TYPE_TRAP      0b1111
#define GATE_TYPE_TASK      0b0101

void IDT_init_entry(u8 vector, void* offset, u16 selector, u8 attributes);
void IDT_set_entry(u8 vector);
void IDT_clear_entry(u8 vector);
void IDT_init();