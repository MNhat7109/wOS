#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

bool timer_set_up_pit(void (*callback)(), u64 ms_per_tick);
void timer_pit_sleep(u64 ms);