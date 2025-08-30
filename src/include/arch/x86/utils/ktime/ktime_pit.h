#pragma once
#include <stdbool.h>
#include "../stdint.h"

bool ktime_set_up_pit();
bool ktime_pit_lazy_disable();
bool ktime_pit_disable();
u16 ktime_pit_read_counter();
u64 ktime_pit_get_freq();

void ktime_pit_sleep(u64 ms);