#pragma once
#include <stdbool.h>

#include "irq_defs.h"

bool irq_set_up_pic();
void irq_pic_setup_int(int interrupt, gsi_handler_t handler, void* ctx);
void irq_pic_disable_int(int interrupt);
void irq_pic_end(int interrupt);
u8 irq_pic_get_vector_cnt();
u8 irq_pic_get_vector_offset();