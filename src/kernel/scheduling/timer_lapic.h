#pragma once
#include <stdbool.h>
#include "../stdint.h"

bool timer_set_up_lapic(void (*callback)(), u64 ms_per_tick);