#pragma once
#include <stdbool.h>
#include "../stdint.h"

#define MAX_DRIVER_ENTRIES 128

typedef enum 
{
    DRIVER_STATE_FAILED,
    DRIVER_STATE_READY,
    DRIVER_STATE_PROBED,
    DRIVER_STATE_UNPROBED,
    DRIVER_STATE_DISABLED,
} generic_driver_state_t;

typedef enum
{
    DRIVER_LOG_ERROR,
    DRIVER_LOG_WARN,
    DRIVER_LOG_NOTICE,
} log_state_t;

struct generic_driver_t
{
    const char* name;
    void (*config)(struct generic_driver_t*);
    void (*probe)(struct generic_driver_t*);
    void (*disable)(struct generic_driver_t*);
    u8 state;
} __attribute__((packed)) ;

typedef const struct generic_driver_t* (*driver_callback_t)();

const struct generic_driver_t* driver_get(const char* name);
void driver_read_state(struct generic_driver_t* driver);
void driver_log_state(struct generic_driver_t* driver, log_state_t level, const char* fmt);
bool driver_run(struct generic_driver_t* driver);

void* driver_alloc(struct generic_driver_t* driver, u32 size);
void driver_free(struct generic_driver_t* driver);

void* driver_memmap(struct generic_driver_t* driver, u32 phys, u32 size /*TODO: Add flags later*/);
void driver_umemmap(struct generic_driver_t* driver, u32 virt, u32 size);

bool driver_terminate(struct generic_driver_t* driver);
void driver_load(driver_callback_t callback);
void driver_unload(const char* name);