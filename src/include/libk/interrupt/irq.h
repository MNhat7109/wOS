#pragma once
#include <libk/interrupt/irq_defs.h>

void IRQ_init();
void IRQ_end(int hw_int);
void IRQ_setup(int hw_int_num, hw_int_handler_t handler, void* ctx);
void* IRQ_get_ctx(int hw_int_num);
void IRQ_disable(int hw_int_num);
void IRQ_enable_interrupts();
void IRQ_disable_interrupts();
void IRQ_halt();