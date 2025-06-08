#pragma once
#include "../../stdint.h"
#include <stdbool.h>

typedef struct
{
    const char* name;
    bool (*probe)();
    u16 (*read_current_counter)();
    void (*config)(u16);
} __attribute__((packed)) pit_driver_t;