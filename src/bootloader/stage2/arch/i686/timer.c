#include "../../drivers/timer.h"
#include "../../drivers/pit.h"
#include "../../stdio.h"

void sleep(u32 ms)
{
    PIT_set_countdown_value(ms);
    PIT_start_countdown();
}

void timer_init()
{
    PIT_init(2000);
    kdebugf(DEBUG_INFO,"TIMER", "Timer is set up\n");
}