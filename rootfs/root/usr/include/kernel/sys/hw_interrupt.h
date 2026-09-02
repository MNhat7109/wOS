#pragma once

typedef struct register_state_t register_state_t;

typedef void (*hw_int_handler_t)(register_state_t* cur_state, void* ctx);

int hw_interrupt_init();

void* hw_interrupt_ctx(int int_no);

void hw_interrupt_ack(int int_no);
int hw_interrupt_register(int int_no, hw_int_handler_t handler, void* ctx);
void hw_interrupt_unregister(int int_no);
void hw_interrupt_disable(int int_no);
void hw_interrupt_enable(int int_no);
void hw_interrupt_enable_all();
void hw_interrupt_disable_all();