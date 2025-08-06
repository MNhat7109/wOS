#pragma once
#include "../driver.h"
#include "../../stdint.h"
#include "cio.h"
#include <stdbool.h>

struct cpu_driver_t
{
    struct generic_driver_t driver_hdr;
    cpu_io_t cpu_io_layer;
    bool (*scan_from_madt)(void (*)(void*));
    bool (*scan_from_mpt)(void (*)(void*));
    u32 (*get_core_id)();
    void (*timer_countdown)();
    u64 (*timer_get_freq)(u64 elapsed_ticks, u64 timer_freq);
    void (*timer_init)(u32, u8);
    // TODO: Add multi-core support
    u32 (*normalize_irq)(u8);
    void (*redirect_gsi)(u8, u8, u8);
    void (*cut_gsi)(u8);
    void (*send_eoi)(u8);
} __attribute__((packed));

const struct generic_driver_t* cpu_get_driver();