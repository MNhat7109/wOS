#pragma once
#include <libk/stdint.h>

struct registers_t;
typedef struct registers_t registers_t;

typedef void (*isr_handler_t)(registers_t* regs, void* ctx);

void ISR_init();
void ISR_reg_handler(u8 vector, isr_handler_t handler, void* ctx);