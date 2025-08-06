#include "driver.h"
#include "../string/string.h"
#include "../stdio.h"
#include <stdarg.h>
#include "../paging/paging.h"

static struct
{
    u32 max_driver_count;
    u32 driver_max_alloc_count;
    u32 item_count;
    volatile struct generic_driver_t** driver_list;
} driver_init_data;

// Need to provide a new metadata to save memory size and stuff
struct generic_driver_list_entry_t
{
    struct generic_driver_mem_t
    {
        void* base;
        u32 size;
    } *memory_info;
    u32 alloc_count;
    struct generic_driver_t* owner;
};

volatile struct generic_driver_t* driver_list[MAX_DRIVER_ENTRIES] = {NULL,};
u32 item_count = 0;

static const char* stat_str[4] = {
    "failed during init phase",
    "is now ready to use",
    "is probed",
    "is not probed (yet)",
    "is disabled"
};

void driver_call_wrapper(struct generic_driver_t* driver, u8 asserted_state, u8 next_state, 
    void (*drv_cb)(struct generic_driver_t*));

void driver_init()
{

}

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

    memcpy(driver_list, new_list, sizeof(new_list));
    item_count=ptr;
}

void driver_unload(const char* name)
{
    bool tampered = false;
    for (u32 i=0;i<item_count; i++)
    {
        if (strcmp(name, driver_list[i]->name) == 0)
        {
            driver_list[i] = NULL;
            tampered = true;
        }
    }

    if (!tampered) return;
    driver_list_condense();
}

bool driver_run(struct generic_driver_t* driver)
{
    if (!driver) return false;

    bool final, running=true;
    while (running)
    {
        driver_read_state(driver);
        switch (driver->state)
        {
            case DRIVER_STATE_READY:
                final = true;
                running = false;
                break;
            case DRIVER_STATE_FAILED:
                final = false;
                running = false;
                break;
            case DRIVER_STATE_DISABLED: 
            case DRIVER_STATE_PROBED: 
                driver_call_wrapper(
                    driver, 
                    DRIVER_STATE_PROBED, 
                    DRIVER_STATE_READY,
                driver->config);
                break;
            case DRIVER_STATE_UNPROBED:
                driver_call_wrapper(
                    driver, 
                    DRIVER_STATE_UNPROBED, 
                    DRIVER_STATE_PROBED,
                driver->probe);
                break;
            default:
                driver->state = DRIVER_STATE_FAILED;
                break;
        }

    }
    return final;
}

void* driver_alloc(struct generic_driver_t* driver, u32 size)
{
    return NULL;
}

void driver_free(struct generic_driver_t* driver)
{

}

void* driver_memmap(struct generic_driver_t* driver, u32 phys, u32 size /*TODO: Add flags later*/)
{
    if (!paging_is_mmu_on()) return (void*)phys;

    u32 page_count = page_convert_from_bytes(size);

    for (u32 i=0;i<page_count;i++)
    page_manager_map_memory(phys+i*PAGE_SIZE, phys+i*PAGE_SIZE);

    return (void*)phys;
}

void driver_umemmap(struct generic_driver_t* driver, u32 virt, u32 size)
{
    if (!paging_is_mmu_on()) return;

    u32 page_count = page_convert_from_bytes(size);

    for (u32 i=0;i<page_count;i++)
    {
        u32 base = page_manager_unmap_memory(virt+i*0x1000);
        if (!base)
        {
            driver_log_state(driver, DRIVER_LOG_ERROR, "Failed to unmap memory" /*At what page?*/);
            return;
        }
    }
}

bool driver_terminate(struct generic_driver_t* driver)
{
    // Here, if the client driver explicitly sets the
    // state to "Failed", return false.

    if (!driver) return false;
    if (driver->state != DRIVER_STATE_READY) return false;

    driver_call_wrapper(driver, DRIVER_STATE_READY, DRIVER_STATE_DISABLED,
    driver->disable);

    if (driver->state == DRIVER_STATE_FAILED) return false;
    return true;
}

void driver_read_state(struct generic_driver_t* driver)
{
    kprintf("Driver: Driver '%s' %s\n", driver->name, stat_str[driver->state]);
}

void driver_log_state(struct generic_driver_t* driver, log_state_t level, const char* fmt)
{
    if (level == DRIVER_LOG_ERROR) driver->state == DRIVER_STATE_FAILED;

    // TODO: After revamping kprintf, hook with the log module (WARN, CRITICAL, etc.)
    kprintf("Driver [%s]: %s\n", driver->name, fmt);
}

void driver_call_wrapper(struct generic_driver_t* driver, u8 asserted_state, u8 next_state, 
    void (*drv_cb)(struct generic_driver_t*))
{
    if (!driver || driver->state != asserted_state)
        return;

    if (!drv_cb)
    {
        driver->state = DRIVER_STATE_FAILED;
        return;
    }

    drv_cb(driver);

    if (driver->state == DRIVER_STATE_FAILED) return;
    driver->state = next_state;
}
