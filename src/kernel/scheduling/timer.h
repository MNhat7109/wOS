#pragma once
#include "../stdint.h"

extern volatile u32 timer_ms;
extern volatile u32 timer_frac_ms;

// void nsleep(u32 ns);
void sleep(u32 ms);
void timer_init();