#include "ktime.h"
#include "ktime_hpet.h"
#include "ktime_pit.h"

#include "../devices/driver.h"
#include "../stdio.h"
#include <stdbool.h>

static struct
{
    bool has_hpet, has_pit;
    bool is_running;
} ktime;

bool ktime_disable()
{
    if (ktime.has_hpet) return ktime_hpet_disable();
    if (ktime.has_pit) return ktime_pit_disable();
    ktime.is_running = false;
    return true; // If source is unknown, it's likely disabled
}

bool ktime_lazy_disable()
{
    if (ktime.has_hpet) return ktime_hpet_lazy_disable();
    if (ktime.has_pit) return ktime_pit_lazy_disable();
    ktime.is_running = false;
    return true; // If source is unknown, it's likely disabled
}

ktime_src_t ktime_get_source()
{
    if (ktime.has_pit) return KTIME_SRC_PIT;
    if (ktime.has_hpet) return KTIME_SRC_HPET;
    return KTIME_SRC_UNKNOWN;
}

bool ktime_is_running()
{
    return ktime.is_running;
}

u64 ktime_get_freq()
{
    if (ktime.has_hpet) return ktime_hpet_get_freq();
    else if (ktime.has_pit) return ktime_pit_get_freq();
    kprintf("ktime: Read frequency failed. API is inactive.\n");
    return 0;
}

u64 ktime_read_counter()
{
    if (ktime.has_hpet) return ktime_hpet_read_counter();
    if (ktime.has_pit) return ktime_pit_read_counter();
    kprintf("ktime: Read counter failed. API is inactive.\n");
    return 0;
}
 
void ktime_sleep(u64 ms)
{
    if (ktime.has_hpet) ktime_hpet_sleep(ms);
    else if (ktime.has_pit) ktime_pit_sleep(ms);
    else kprintf("ktime: Sleep failed. API is inactive.\n");
}

void ktime_init()
{
    ktime.is_running = false;
    ktime.has_hpet = ktime_set_up_hpet();
    if (ktime.has_hpet)
    {
        kprintf("ktime: HPET will be used.\n");
        driver_unload("i8254 PIT");
        ktime.has_pit = false;
    }
    else
    {
        ktime.has_pit = ktime_set_up_pit();
        if (ktime.has_pit)
        kprintf("ktime: i8254 PIT will be used.\n");
    }

    if (!ktime.has_hpet && !ktime.has_pit)
    {
        kprintf("ktime: Init failed. Timer will be disabled.\n");
        return;
    }

    ktime.is_running = true;
}