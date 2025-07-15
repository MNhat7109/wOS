#pragma once
#include "../driver.h"
#include "../pio.h"

#define PIT_C0_DATA_PORT 0x40
#define PIT_C1_DATA_PORT 0x41
#define PIT_C2_DATA_PORT 0x42
#define PIT_CMD_PORT 0x43

struct pit_driver_t
{
    struct generic_driver_t driver_hdr;
    pio_layer_t pio_utils;
    u16 reload_value;
    u16 (*read_counter)(struct pit_driver_t*);
} __attribute__((packed));

const struct generic_driver_t* i8254_get_driver();