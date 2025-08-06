#pragma once
#include <stdbool.h>

#include "irq_defs.h"

bool irq_set_up_cpu();
void irq_cpu_setup_int(int interrupt, gsi_handler_t handler, void* ctx);
void irq_cpu_disable_int(int interrupt);
void irq_cpu_end(int interrupt);
u8 irq_cpu_get_vector_cnt();
u8 irq_cpu_get_vector_offset();
u8 irq_cpu_get_handler_offset();