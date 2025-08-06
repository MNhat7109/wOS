#include "timer.h"
#include "timer_lapic.h"
#include "timer_pit.h"

#include <stdbool.h>
#include "../stdio.h"

#define TIMER_INT_VECTOR 0x20
#define DEFAULT_MS_PER_TICK 1

static struct
{
    bool has_cpu, has_pit;
    bool is_running;
    scheduling_call cb_fp;
} timer;

void timer_set_callback(scheduling_call callback)
{
    timer.cb_fp = callback;
}

void timer_init()
{
    timer.is_running = false;
    timer.cb_fp = NULL;
    timer.has_cpu = timer_set_up_lapic(timer.cb_fp, DEFAULT_MS_PER_TICK);
    if (timer.has_cpu)
    {
        kprintf("Scheduler: LAPIC will be used as a timer\n");
        timer.has_pit = false;
        // No driver_unload PIT here
        // What if LAPIC runs based off of ktime's PIT? (No HPET situation)
    }
    else
    {
        kprintf("Scheduler: LAPIC failed, will use PIT instead...\n");
        timer.has_pit = timer_set_up_pit(timer.cb_fp, DEFAULT_MS_PER_TICK);
        if (timer.has_pit) kprintf("Scheduler: PIT will be used as a timer\n");
    }

    if (!timer.has_cpu && !timer.has_pit)
    {
        kprintf("Scheduler: Timer init() failed.\n");
        return;
    }

    timer.is_running = true;
}

bool timer_is_running()
{
    return timer.is_running;
}

void timer_sleep(u64 ms)
{
    if (timer.has_pit) timer_pit_sleep(ms);
}