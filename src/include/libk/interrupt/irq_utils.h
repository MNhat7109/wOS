#pragma once
#include "irq_defs.h"

void irq_load_handler(int interrupt, gsi_handler_t handler, void* ctx);
void irq_unload_handler(int interrupt);
