#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    int type;
    u32 address;
} __attribute__((packed)) system_desc_ptr_t;

bool RSDP_scan(system_desc_ptr_t** address);