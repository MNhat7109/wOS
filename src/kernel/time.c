#include "time.h"
#include "scheduling/timer.h"
#include "ktime/ktime.h"
#include "stdio.h"

void usleep(u64 ms)
{
    if (ktime_is_running()) ktime_sleep(ms);
    else if (timer_is_running()) timer_sleep(ms);
    else
    {
        kprintf("usleep: No timer available. System will now halt\n");
        for (;;);
    }
}

void sleep(u64 sec)
{
    usleep(sec*1000);
}