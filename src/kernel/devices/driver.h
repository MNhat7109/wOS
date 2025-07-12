#pragma once
#include <stdbool.h>
#include "../stdint.h"

#define MAX_DRIVER_ENTRIES 128

struct generic_driver_t
{
    const char* name;
    void (*config)(struct generic_driver_t*);
    bool (*probe)(struct generic_driver_t*);
    u64 (*read)();
    void (*write)(u32 offset, u64 value);
    void (*disable)(struct generic_driver_t*);
} __attribute__((packed)) ;

extern volatile struct generic_driver_t* driver_list[MAX_DRIVER_ENTRIES];
typedef const struct generic_driver_t* (*driver_callback_t)();

const struct generic_driver_t* driver_get(const char* name);
void driver_load(driver_callback_t callback);
void driver_unload(const char* name);