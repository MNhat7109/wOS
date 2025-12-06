#include "timer.h"
#include "pit.h"

void sleep(u32 ms)
{
    PIT_set_countdown_value(ms);
    PIT_start_countdown();
}

void timer_init()
{
    PIT_init(2000);
}