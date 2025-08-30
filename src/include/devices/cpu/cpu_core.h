#pragma once
#include <devices/driver.h>
#include <libk/stdint.h>
#include <stdbool.h>

struct cpu_core_ops_t;
struct cpu_timer_ops_t;

struct cpu_core_driver_ops_t
{
    struct generic_driver_ops_t ops_hdr;
    const struct cpu_core_ops_t* core_ops;
    const struct cpu_timer_ops_t* timer_ops;
};

const struct generic_driver_ops_t* cpu_core_get_driver_ops();