#pragma once
#include <kernel/arch/i686/state.h>

typedef void (*isr_handler_cb_t)(register_state_t* reg_state, void* ctx);

void isr_init();
void isr_register_handler(u8 vector, isr_handler_cb_t handler, void* handler_ctx);