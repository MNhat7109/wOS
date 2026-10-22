#pragma once
#include <stdint.h>
#include <stdbool.h>

#include <kernel/interrupt/interrupt_vector.h>

#define MODULE_IRQ "IRQ"

typedef struct int_ops_t
{
    void (*reg)(int irq, int_dispatcher_cb_t cb, void* ctx);
    void (*unreg)(int irq, int_dispatcher_cb_t cb);
} irq_ops_t;

void irq_init();

void irq_register(u8 irq, int_dispatcher_cb_t cb, void* ctx);
void irq_unregister(u8 irq, int_dispatcher_cb_t cb);
void irq_ack(u8 irq);