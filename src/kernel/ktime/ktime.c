#include "ktime.h"
#include "../devices/hpet/hpet.h"
#include "../devices/driver.h"
#include "../x86/x86.h"
#include "../stdio.h"

#define UINT32_MAX 0xFFFFFFFFU

u64 hpet_freq;

const generic_driver_t* hpet_generic_driver = NULL;

u64 ktime_get_freq()
{
    return hpet_freq;
}

u64 ktime_read_counter()
{
    hpet_generic_driver->write(DRIVER_CMD, HPET_DRV_CMD_SEND_MMIO_OFFSET);
    hpet_generic_driver->write(DRIVER_DATA, 0xF0);
    hpet_generic_driver->write(DRIVER_CMD, HPET_DRV_CMD_RECIEVE_MMIO_LO);
    u32 cnt_lo = hpet_generic_driver->read();
    hpet_generic_driver->write(DRIVER_CMD, HPET_DRV_CMD_RECIEVE_MMIO_HI);
    u32 cnt_hi = hpet_generic_driver->read();
    return cnt_lo | (cnt_hi<<32);
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
    hpet_generic_driver = driver_get("HPET");
    if (!hpet_generic_driver)
    {
        kprintf("Timer: HPET not found\n");
        return;
    } 
    hpet_generic_driver->config();

    hpet_generic_driver->write(DRIVER_CMD, HPET_DRV_CMD_SEND_MMIO_OFFSET);
    hpet_generic_driver->write(DRIVER_DATA, 0x00);
    hpet_generic_driver->write(DRIVER_CMD, HPET_DRV_CMD_RECIEVE_MMIO_HI);
    u32 fs_per_tick =hpet_generic_driver->read();
    hpet_freq = 1000000000000000ULL / fs_per_tick;
}