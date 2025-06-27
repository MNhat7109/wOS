#pragma once
#include "isr.h"

#define MAX_HANDLED_GSI 64

typedef void (*gsi_handler_t)(registers_t*);
extern gsi_handler_t gsi_handler_table[MAX_HANDLED_GSI];

#define APIC_REMAP_OFFSET 0x20

void IRQ_init();
void IRQ_reg_handler(int gsi, gsi_handler_t handler);
void IRQ_end();
void IRQ_setup(int gsi_num, gsi_handler_t handler);