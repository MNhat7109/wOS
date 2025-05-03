#pragma once
#include "../stdint.h"

extern u32 timer_frac, timer_ms;
void sleep(u32 ms);
void PIT_init(u32 reload_value);