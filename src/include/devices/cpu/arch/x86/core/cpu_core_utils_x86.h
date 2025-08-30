#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

struct cio_layer_t;
typedef struct cio_layer_t cio_layer_t;

bool cpu_core_is_bsp(cio_layer_t* cpu_io);
bool cpu_core_x2apic_enabled(cio_layer_t* cpu_io);
void cpu_core_enable_x2apic(cio_layer_t* cpu_io);
void cpu_core_enable_xapic(cio_layer_t* cpu_io);
u32 cpu_core_get_bsp_runtime_lapic_id(cio_layer_t* cpu_io);
