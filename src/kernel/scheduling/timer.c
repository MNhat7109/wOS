#include "timer.h"
#include "../hal/cpu/lapic.h"
#include "../hal/interrupt/isr.h"
#include "../ktime/ktime.h"

#define TIMER_INT_VECTOR 0x20

u64 timer_recalibrate()
{
    LAPIC_timer_init(false, 0x0, 0xFFFFFFFF, LAPIC_TIMER_MODE_ONE_SHOT, LAPIC_TIMER_DIVIDE_16);
    
    u64 start = ktime_read_counter();
    sleep(10); // Sleeps in ms
    u64 end = ktime_read_counter();

    u32 lapic_current = LAPIC_read(LAPIC_REG_CURRCNT);
    u32 lapic_elapsed = 0xFFFFFFFF - lapic_current;
    u64 ktime_elapsed = end-start;

    u64 elapsed_ns = (ktime_elapsed*1000000000ULL)/ktime_get_freq();

    u64 lapic_freq = (lapic_elapsed*1000000000ULL)/elapsed_ns;
    return lapic_freq;
}

void timer_handler(registers_t* regs)
{
    // TODO: Do fancy stuffs for multitasking
    LAPIC_send_eoi();
}

void timer_init()
{
    u32 freq = timer_recalibrate();
    u32 tick_cnt = freq / 100;

    LAPIC_timer_init(true, TIMER_INT_VECTOR, tick_cnt, LAPIC_TIMER_MODE_PERIODIC, LAPIC_TIMER_DIVIDE_16);
    ISR_reg_handler(TIMER_INT_VECTOR, timer_handler);
}