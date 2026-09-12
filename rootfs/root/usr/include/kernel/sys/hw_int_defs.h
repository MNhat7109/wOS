#pragma once
#include <kernel/sys/hw_interrupt.h>
#include <ds/list.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct hw_int_wrapper_t
{
    int interrupt_num;
    hw_int_handler_t handler;
    void* ctx;
} hw_int_wrapper_t;

struct hw_int_data_t;

typedef struct hw_int_ops_t
{
    void (*reg)(int no, hw_int_handler_t handler, void* ctx);
    void (*unreg)(int no);
    void (*ack)(int no);
    void (*enable)(int no);
    void (*disable)(int no);
    void (*enable_all)();
    void (*disable_all)();
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
    .interrupt_num = 0, \
    .handler = NULL, \
    .ctx = NULL \
}