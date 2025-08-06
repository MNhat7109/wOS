#pragma once
#include <stdbool.h>
#include "../stdint.h"

bool ktime_set_up_hpet();
bool ktime_hpet_lazy_disable(); // Just in case
bool ktime_hpet_disable();
u64 ktime_hpet_read_counter();
u64 ktime_hpet_get_freq();

void ktime_hpet_sleep(u64 ms);