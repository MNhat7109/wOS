#pragma once
#include "../../stdint.h"

typedef struct
{
    u32 ds;
    u32 edi, esi,  ebp, _, ebx, edx, ecx, eax;
    u32 vector, error;
    u32 eip, cs, eflags, esp, ss; 
} __attribute__((packed)) registers_t;

typedef void (*isr_handler_t)(registers_t* regs);

void ISR_init();
void ISR_reg_handler(u8 vector, isr_handler_t handler);