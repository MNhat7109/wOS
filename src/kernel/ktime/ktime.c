#include "ktime.h"
#include "../devices/hpet/hpet.h"
#include "../devices/driver.h"
#include "../x86/x86.h"
#include "../stdio.h"

#define UINT32_MAX 0xFFFFFFFFU

u64 hpet_freq;
u32 hpet_phys;

const struct hpet_driver_t* hpet_generic_driver = NULL;

u64 ktime_get_freq()
{
    return hpet_freq;
}

u64 ktime_read_counter()
{
    return hpet_generic_driver->mmio_utils.readq(hpet_phys, 0xF0);
}

void sleep(u64 ms)
{
    u64 ticks = (hpet_freq*ms)/1000;
    u64 start=ktime_read_counter();
    while ((ktime_read_counter()-start) < ticks);
}

void ktime_init()
{
    driver_load(hpet_get_driver);
    hpet_generic_driver = (struct hpet_driver_t*)driver_get("HPET");
    if (!hpet_generic_driver)
    {
        kprintf("Timer: HPET not found\n");
        return;
    } 

    if (!hpet_generic_driver->driver_hdr.probe((struct generic_driver_t*)hpet_generic_driver))
    {
        kprintf("Timer: HPET is buggy or faulty\n");
        return;
    }
    
    hpet_generic_driver->driver_hdr.config((struct generic_driver_t*)hpet_generic_driver);
    hpet_phys = hpet_generic_driver->hpet_acpi_table->hpet_address.base;

    u32 fs_per_tick=hpet_generic_driver->mmio_utils.readq(hpet_phys, 0x00)>>32;
    hpet_freq = 1000000000000000ULL / fs_per_tick;
}