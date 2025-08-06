#pragma once
#include "irq_defs.h"

void IRQ_init();
void IRQ_end(int gsi);
void IRQ_setup(int gsi_num, gsi_handler_t handler, void* ctx);
void* IRQ_get_ctx(int gsi_num);
void IRQ_disable(int gsi_num);
void IRQ_enable_interrupts();
void IRQ_disable_interrupts();
void IRQ_halt();