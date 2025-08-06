#include "timer_pit.h"

#include "../hal/interrupt/irq.h"
#include "../devices/i8254/i8254.h"
#include "../ktime/ktime.h"
#include "../stdio.h"

static struct 
{
    struct pit_driver_t* pit_dev;
    void (*schedule_callback)();
    u64 ticks_since_boot;
} timer_pit;

#define PIT_FREQ 1193182ULL

static void timer_pit_set_callback(void (*callback)());
static void timer_pit_handler(registers_t* regs, void* ctx);

bool timer_set_up_pit(void (*callback)(), u64 ms_per_tick)
{
    // Disable ktime first, if ktime uses it
    // But we should do it lazily, as doing so will prevent the PIT driver from being nuked
    if (ktime_get_source() == KTIME_SRC_PIT)
    {
        kprintf("Scheduler: ktime uses PIT, conflict detected. Disabling ktime...\n");
        if (!ktime_lazy_disable())
        {
            kprintf("Scheduler: Cannot disable ktime\n");
            return false;
        }
    }

    // Set up our own PIT
    timer_pit.pit_dev = (struct pit_driver_t*)driver_get("i8254 PIT");
    if (!timer_pit.pit_dev)
    {
        kprintf("Scheduler: i8254 PIT driver not found\n");
        return false;
    }

    // Feed the params first, then run
    timer_pit.pit_dev->reload_value = PIT_FREQ*ms_per_tick/1000;
    if (!driver_run((struct generic_driver_t*)timer_pit.pit_dev))
    {
        kprintf("Scheduler: i8254 PIT driver failed to start\n");
        driver_unload("i8254 PIT"); // Now, we officially unload the PIT
        return false;
    }

    timer_pit_set_callback(callback);

    // Enable IRQs for the PIT and Register the handler
    IRQ_setup(0, timer_pit_handler, NULL);
    return true; // Profit
}

void timer_pit_sleep(u64 ms)
{
    // Cap the starting tick
    u64 tick_start = timer_pit.ticks_since_boot;

    u64 total_ticks = ms*PIT_FREQ/1000;
    while (timer_pit.ticks_since_boot-tick_start < total_ticks) IRQ_halt();
}

/*-- STATIC FUNCTIONS --*/

static void timer_pit_set_callback(void (*callback)())
{
    timer_pit.schedule_callback = callback;
}

static void timer_pit_handler(registers_t* regs, void* ctx)
{
    (void)ctx;

    if (timer_pit.schedule_callback)
    timer_pit.schedule_callback();

    timer_pit.ticks_since_boot++;
    IRQ_end(0);
}