#pragma once
#include "../../stdint.h"
#include <stdbool.h>

typedef struct
{
    const char* name;
    void (*config)(u8, u8);
    void (*mask)(int);
    void (*unmask)(int);
    void (*send_eoi)(int);
    void (*disable)();
    bool (*probe)();
    u16 (*read_irr)();
    u16 (*read_isr)();
} __attribute__((packed)) pic_driver_t;
