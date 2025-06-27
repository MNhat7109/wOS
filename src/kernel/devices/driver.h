#pragma once
#include <stdbool.h>
#include "../stdint.h"

#define MAX_DRIVER_ENTRIES 128

#define DRIVER_CMD 0
#define DRIVER_DATA 1

typedef struct
{
    const char* name;
    void (*config)();
    bool (*probe)();
    u32 (*read)();
    void (*write)(int pool, u32 value);
    void (*disable)();
} __attribute__((packed)) generic_driver_t;

typedef struct
{
    u32 pool_value;
    u16 cmd_sig;
    bool send, receive;
} __attribute__((packed)) generic_driver_io_t;

extern volatile generic_driver_t* driver_list[MAX_DRIVER_ENTRIES];
typedef const generic_driver_t* (*driver_callback_t)();

const generic_driver_t* driver_get(const char* name);
void driver_load(driver_callback_t callback);
void driver_unload(const char* name);