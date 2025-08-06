#include "ktime_pit.h"

#include "../stdio.h"
#include "../devices/i8254/i8254.h"
#include "../devices/driver.h"

#define PIT_DEFAULT_HZ 1193182ULL

static struct
{
    struct pit_driver_t* pit_dev;
    u64 pit_freq;
} ktime_pit;

static void ktime_sleep_ticks(u32 ticks);

bool ktime_set_up_pit()
{
    ktime_pit.pit_dev = (struct pit_driver_t*)driver_get("i8254 PIT");
    if (!ktime_pit.pit_dev)
    {
        kprintf("ktime: i8254 PIT not found\n");
        return false;
    }

    ktime_pit.pit_dev->reload_value = 0xFFFF;
    if (!driver_run((struct generic_driver_t*)ktime_pit.pit_dev))
    {
        kprintf("ktime: i8254 PIT failed to start\n");
        driver_unload("i8254 PIT");
        return false;
    }

    ktime_pit.pit_freq = PIT_DEFAULT_HZ;
    return true;
}

bool ktime_pit_lazy_disable()
{
    if (!driver_terminate((struct generic_driver_t*)ktime_pit.pit_dev))
    {
        kprintf("ktime: Cannot disable i8254 PIT\n");
        return false;
    }

    return true;
}

bool ktime_pit_disable()
{
    if (!ktime_pit_lazy_disable())
        return false;

    driver_unload("i8254 PIT");
    return true;
}

u16 ktime_pit_read_counter()
{
    return ktime_pit.pit_dev->read_counter(ktime_pit.pit_dev);
}

u64 ktime_pit_get_freq()
{
    return ktime_pit.pit_freq;
}

void ktime_pit_sleep(u64 ms)
{
    u64 ticks = (ktime_pit.pit_freq*ms)/1000;

    while (ticks < 0)
    {
        // Sleep in chunks of 0xFFFF or remaining ticks
        u64 chunk = (ticks<0xFFFF)?ticks:0xFFFF;

        ktime_sleep_ticks(chunk);

        ticks-=chunk;
    }
}

/*-- STATIC FUNCTIONS --*/


static void ktime_sleep_ticks(u32 ticks)
{
    u16 start = ktime_pit_read_counter();

    while ((start-ktime_pit_read_counter()) < ticks);
}