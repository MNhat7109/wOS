#pragma once
#include "../../../stdint.h"
#include <stdbool.h>

struct lapic_driver_t;

void lapic_write(struct lapic_driver_t* self, u32 offset, u32 value);
u32 lapic_read(struct lapic_driver_t* self, u32 offset);
u8 lapic_cook_div_mode(u8 mode);