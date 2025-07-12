#include "driver.h"
#include "../string/string.h"
#include "../stdio.h"

volatile struct generic_driver_t* driver_list[MAX_DRIVER_ENTRIES] = {NULL,};
u32 item_count = 0;

const struct generic_driver_t* driver_get(const char* name)
{
    for (u32 i=0;i<item_count;i++)
    {
        struct generic_driver_t* driver = driver_list[i];
        if (strcmp(name, driver->name) == 0)
            return driver;
    }
    return NULL;
}

void driver_load(driver_callback_t callback)
{
    if (item_count >= MAX_DRIVER_ENTRIES) return;
    const struct generic_driver_t* driver = callback();
    if (!driver) return;
    driver_list[item_count++] = driver;
}

void driver_list_condense()
{
    struct generic_driver_t* new_list[MAX_DRIVER_ENTRIES] = {NULL,};
    u32 ptr=0;
    for (u32 i=0;i<item_count; i++)
    {
        if (driver_list[i]) new_list[ptr++] = driver_list[i];
    }

    memcpy(driver_list, new_list, MAX_DRIVER_ENTRIES);
    item_count=ptr;
}

void driver_unload(const char* name)
{
    for (u32 i=0;i<item_count; i++)
    {
        if (strcmp(name, driver_list[i]->name) == 0)
        {
            driver_list[i] = NULL;
        }
    }

    driver_list_condense();
}