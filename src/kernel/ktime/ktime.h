#pragma once
#include "../stdint.h"

void sleep(u64 ms);
void ktime_init();
u64 ktime_read_counter();
u64 ktime_get_freq();