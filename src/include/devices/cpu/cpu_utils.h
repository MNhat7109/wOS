#pragma once
#include <libk/stdint.h>
#include <stdbool.h>
#include <devices/cpu/cpu_defs.h>

struct generic_driver_tree_node_t;

struct cpu_interrupt_ops_t;

struct generic_driver_tree_node_t* cpu_create_core_node(
    struct generic_driver_tree_node_t* cpu_self,
    u32 index
);
void cpu_enumerate_cores(
    struct generic_driver_tree_node_t* cpu_self
);

const struct cpu_interrupt_ops_t* cpu_load_int_ops();
bool cpu_scan_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_drv_node,
    table_callback_t callback,
    void* ctx
);
void cpu_config_new_features(
    struct generic_driver_tree_node_t* self
);
void cpu_config_table(
    struct generic_driver_tree_node_t* self,
    struct generic_driver_tree_node_t* acpi_drv_node
);