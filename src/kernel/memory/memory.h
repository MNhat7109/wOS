#pragma once
#include "../stdint.h"

typedef struct
{
    u32 entries_count;
    struct
    {
        u64 base, length;
        u32 type;
        u32 acpi;
    } __attribute__((packed)) regions[];
} __attribute__((packed)) memory_info_t;

// TODO: Add some functions to sort, create, merge regions