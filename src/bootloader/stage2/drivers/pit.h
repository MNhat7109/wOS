#pragma once
#include "../stdint.h"

void PIT_init(u32 reload_value);
void PIT_set_countdown_value(u32 value);
void PIT_start_countdown();