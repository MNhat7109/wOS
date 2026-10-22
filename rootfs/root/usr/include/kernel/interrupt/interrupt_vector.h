#pragma once

#define MODULE_INT "INTERRUPT"

typedef enum
{
    INT_STATUS_HANDLED,
    INT_STATUS_NOTMINE, // Not My Interrupt
} int_dispatcher_status_t;

typedef struct register_state_t register_state_t;

typedef int (*int_dispatcher_cb_t)(register_state_t* regs, void* ctx);

typedef struct int_dispatcher_ops_t
{
    void (*enable_int)(int irq);
    void (*disable_int)(int irq);
    void (*ack)(int irq);
} int_dispatcher_ops_t;

typedef struct int_dispatch_blk_t
{
    int_dispatcher_cb_t dispatcher;
    int interrupt_num;
    void* ctx;
    int_dispatcher_ops_t* ops;
} int_dispatch_blk_t;

void interrupt_vector_init();
int interrupt_vector_alloc();
int_dispatch_blk_t* interrupt_vector_find_dispatcher(int vector, int_dispatcher_cb_t dispatcher);
int interrupt_vector_populate(int min_vector, int int_num, int_dispatcher_ops_t* ops, int_dispatcher_cb_t dispatcher, void* ctx);
void interrupt_vector_unlink_dispatcher(int vector, int_dispatcher_cb_t dispatcher);

