#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

bool timer_set_up_lapic(void (*callback)(), u64 ms_per_tick);