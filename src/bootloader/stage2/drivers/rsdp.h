#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct system_desc_ptr_t
{
    int type;
    u32 address;
} __attribute__((packed)) system_desc_ptr_t;

bool rsdp_scan(system_desc_ptr_t** address_out);