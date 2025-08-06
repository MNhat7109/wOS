#pragma once
#include <stdbool.h>
#include "../stdint.h"

typedef void (*scheduling_call)(void);

void timer_set_callback(scheduling_call callback);
bool timer_is_running();
void timer_sleep(u64 ms);
void timer_init();