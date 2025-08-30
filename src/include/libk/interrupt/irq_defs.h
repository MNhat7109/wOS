#pragma once
#include <libk/stdint.h>

#define MAX_HANDLED_INT 0x60

struct registers_t;
typedef struct registers_t registers_t;

typedef void (*hw_int_handler_t)(registers_t*, void*);
typedef struct
{
    hw_int_handler_t handler;
    void* ctx;
} int_handle_wrapper;

extern u8 handler_offset;
extern int_handle_wrapper hw_int_handler_table[MAX_HANDLED_INT];