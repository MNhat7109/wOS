#include "timer.h"
#include "../hal/cpu/lapic.h"
#include "../ktime/ktime.h"

u64 timer_recalibrate()
{
    LAPIC_timer_init(0x20, 0xFFFFFFFF, LAPIC_TIMER_MODE_ONE_SHOT, LAPIC_TIMER_DIVIDE_16);
    
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

void timer_init()
{

}