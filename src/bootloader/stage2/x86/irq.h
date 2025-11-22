#pragma once
#include "isr.h"

typedef void (*irq_handler_t)(registers_t* regs);

void IRQ_init();
void IRQ_reg_handler(int irq, irq_handler_t handler);

void __attribute__((cdecl)) i686_panic();
void __attribute__((cdecl)) i686_enable_interrupt();
void __attribute__((cdecl)) i686_disable_interrupt();
void __attribute__((cdecl)) i686_halt();