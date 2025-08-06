#include "ktime_hpet.h"

#include "../stdio.h"
#include "../devices/hpet/hpet.h"
#include "../devices/driver.h"

#define HPET_64BIT_CNTER (1<<13)

static struct
{
    struct hpet_driver_t* hpet_dev;
    u64 hpet_freq;
    u64 hpet_cap;
    u32 hpet32_last_cnt;
    u32 hpet32_hi_cnt;
} ktime_hpet;

u64 ktime_32bit_extension();

bool ktime_set_up_hpet()
{
    ktime_hpet.hpet_dev = (struct hpet_driver_t*)driver_get("HPET");
    if (!ktime_hpet.hpet_dev)
    {
        kprintf("ktime: HPET not found\n");
        return false;
    }

    if (!driver_run((struct generic_driver_t*)ktime_hpet.hpet_dev))
    {
        kprintf("ktime: HPET failed to start\n");
        driver_unload("HPET");
        return false;
    }
    
    ktime_hpet.hpet_cap = ktime_hpet.hpet_dev->read(ktime_hpet.hpet_dev, 0x00);
    u32 tick_fs = ktime_hpet.hpet_cap>>32;

    if (tick_fs == 0) 
    {
        kprintf("ktime: HPET reported invalid tick_fs=0x0\n");
        driver_unload("HPET");
        return false;
    }

    ktime_hpet.hpet32_hi_cnt=0;
    ktime_hpet.hpet32_last_cnt=0;

    ktime_hpet.hpet_freq = (1000ULL*1000ULL*1000ULL*1000ULL*1000ULL) / tick_fs;
    return true;
}

bool ktime_hpet_lazy_disable()
{
    if (!driver_terminate((struct generic_driver_t*)ktime_hpet.hpet_dev))
    {
        kprintf("ktime: Cannot disable HPET\n");
        return false;
    }
    return true;
}

bool ktime_hpet_disable()
{
    if (!ktime_hpet_lazy_disable())
        return false;

    driver_unload("HPET");
    return true;
}

u64 ktime_hpet_read_counter()
{
    if (ktime_hpet.hpet_cap & HPET_64BIT_CNTER)
        return ktime_hpet.hpet_dev->read(ktime_hpet.hpet_dev, 0xF0);
    return ktime_32bit_extension();
}

u64 ktime_hpet_get_freq()
{
    return ktime_hpet.hpet_freq;
}

void ktime_hpet_sleep(u64 ms)
{
    u64 ticks = (ktime_hpet.hpet_freq*ms)/1000;
    u64 start=ktime_hpet_read_counter();
    while ((ktime_hpet_read_counter()-start) < ticks);
}


u64 ktime_32bit_extension()
{
    u32 current_lo = ktime_hpet.hpet_dev->read32(ktime_hpet.hpet_dev, 0xF0);
    if (current_lo < ktime_hpet.hpet32_last_cnt) ktime_hpet.hpet32_hi_cnt++;

    ktime_hpet.hpet32_last_cnt = current_lo;
    return (((u64)ktime_hpet.hpet32_hi_cnt<<32) | current_lo);
}