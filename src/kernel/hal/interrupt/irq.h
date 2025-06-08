#pragma once
#include "isr.h"
#include "../../devices/i8259/pic.h"

typedef void (*irq_handler_t)(registers_t*);

extern const pic_driver_t* pic_driver;

void IRQ_init();
void IRQ_reg_handler(int irq, irq_handler_t handler);