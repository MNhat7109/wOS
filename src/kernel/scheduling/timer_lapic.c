#include "timer_lapic.h"

#include "../hal/interrupt/isr.h"
#include "../devices/cpu/cpu.h"
#include "../ktime/ktime.h"
#include "../stdio.h"

#define LAPIC_TIMER_OFFSET 0x20

static struct
{
    struct cpu_driver_t* cpu_dev;
    void (*schedule_callback)();
} timer_lapic;

static void timer_lapic_set_callback(void (*callback)());
static void timer_lapic_handler(registers_t* regs);

bool timer_set_up_lapic(void (*callback)(), u64 ms_per_tick)
{
    timer_lapic.cpu_dev = (struct cpu_driver_t*)driver_get("CPU");
    if (!timer_lapic.cpu_dev)
    {
        kprintf("Scheduler: CPU driver not found\n");
        return false;
    }
    if (!driver_run((struct generic_driver_t*)timer_lapic.cpu_dev))
    {
        kprintf("Scheduler: CPU driver failed to start\n");
        driver_unload("CPU");
        return false;
    }

    // Init one-shot mode
    timer_lapic.cpu_dev->timer_countdown();

    u64 start = ktime_read_counter();
    sleep(ms_per_tick);
    u64 end = ktime_read_counter();
    u64 elapsed_ticks = end-start; // TODO: PIT support

    u64 elapsed_time = (elapsed_ticks*1000000000ULL)/ktime_get_freq();
    u64 lapic_freq = timer_lapic.cpu_dev->timer_get_freq(elapsed_time, 1000000000ULL);

    u32 timer_ticks = lapic_freq*ms_per_tick / 1000;
    // Hz -> cycles/ticks per second
    // Here lapic_freq is measured in ms_per_tick, so to get the ticks,
    // multiply by ms per tick, and divide the frequency by 1000ms (1 second)

    timer_lapic.cpu_dev->timer_init(timer_ticks, LAPIC_TIMER_OFFSET);
    // Here, if the CPU driver is ready, then interrupts will have been
    // routed to the APIC and not the PIC, leaving offset 0x20-0x2F free for use
    // which we'll pick vector 0x20

    timer_lapic_set_callback(callback);
    
    // Register the handler
    ISR_reg_handler(LAPIC_TIMER_OFFSET, timer_lapic_handler);
    return true;
}


static void timer_lapic_handler(registers_t* regs)
{
    if (timer_lapic.schedule_callback)
    timer_lapic.schedule_callback();
    timer_lapic.cpu_dev->send_eoi(0xDEADBEEF);
    // Could've put anything in here, LAPIC won't care
}

static void timer_lapic_set_callback(void (*callback)())
{
    timer_lapic.schedule_callback = callback;
}
