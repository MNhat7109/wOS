#pragma once
#include "isr.h"

typedef void (*irq_handler_t)(registers_t*);

void IRQ_init();
void IRQ_reg_handler(int irq, irq_handler_t handler);