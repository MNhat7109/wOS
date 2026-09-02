#pragma once
#include <kernel/sys/hw_interrupt.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct hw_int_wrapper_t
{
    hw_int_handler_t handler;
    void* ctx;
} hw_int_wrapper_t;

struct hw_int_data_t;

typedef struct hw_int_ops_t
{
    int (*vector2intno)(struct hw_int_data_t*, int vector);
    int (*init)(struct hw_int_data_t*);
    void (*ack)(struct hw_int_data_t*, int no);
    void (*enable)(struct hw_int_data_t*, int no);
    void (*disable)(struct hw_int_data_t*, int no);
    void (*enable_all)(struct hw_int_data_t*);
    void (*disable_all)(struct hw_int_data_t*);
} hw_int_ops_t;

typedef struct hw_int_data_t
{
    usize handler_cnt;
    hw_int_wrapper_t* hw_int_handler_table;
    bool init;
    const hw_int_ops_t* int_ops;
} hw_int_data_t;


void hw_default_handler(register_state_t* regs, void* ctx);

#define HANDLER_DEFAULT (hw_int_wrapper_t){ \
    .handler = &hw_default_handler, \
    .ctx = NULL \
}