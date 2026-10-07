#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MODULE_IRQ "IRQ"

typedef enum
{
    IRQ_STATUS_HANDLED,
    IRQ_STATUS_NOTMINE, // Not My Interrupt
} irq_dispatcher_status_t;

typedef struct register_state_t register_state_t;

typedef int (*irq_dispatcher_cb_t)(register_state_t* regs, void* ctx);

typedef struct irq_ops_t
{
    void (*reg)(u8 irq, irq_dispatcher_cb_t cb, void* ctx);
    void (*unreg)(u8 irq, irq_dispatcher_cb_t cb);
    void (*ack)(u8 irq);
} irq_ops_t;

typedef struct irq_dispatch_blk_t
{
    int type;
    irq_dispatcher_cb_t dispatcher;
    int irq;
    void* ctx;
} irq_dispatch_blk_t;

struct irq_data_t
{
    const irq_ops_t* cur_ops;
    bool done_setup;
};

void irq_init();

void irq_load_ops(const irq_ops_t* ops);

u8 irq_vector_alloc();
irq_dispatch_blk_t* irq_vector_find(int type, u8 vector, irq_dispatcher_cb_t dispatcher);
int irq_vector_populate(int type, u8 vector_start, int irq, irq_dispatcher_cb_t dispatcher, void* ctx);
void irq_vector_free(int type, u8 vector, irq_dispatcher_cb_t dispatcher);

void irq_register(u8 irq, irq_dispatcher_cb_t cb, void* ctx);
void irq_unregister(u8 irq, irq_dispatcher_cb_t cb);
void irq_ack(u8 irq);