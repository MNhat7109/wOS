#pragma once
#include "../../stdint.h"

#define MAX_HANDLED_INT 0x60

struct registers_t;
typedef struct registers_t registers_t;

typedef void (*gsi_handler_t)(registers_t*, void*);
typedef struct
{
    gsi_handler_t handler;
    void* ctx;
} int_handle_wrapper;

extern u8 handler_offset;
extern int_handle_wrapper gsi_handler_table[MAX_HANDLED_INT];