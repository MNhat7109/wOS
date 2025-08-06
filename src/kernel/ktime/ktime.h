#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef enum
{
    KTIME_SRC_HPET,
    KTIME_SRC_PIT,
    KTIME_SRC_UNKNOWN
} ktime_src_t;

void ktime_sleep(u64 ms);
void ktime_init();
u64 ktime_read_counter();
u64 ktime_get_freq();
bool ktime_disable();
bool ktime_lazy_disable();
bool ktime_is_running();
ktime_src_t ktime_get_source();