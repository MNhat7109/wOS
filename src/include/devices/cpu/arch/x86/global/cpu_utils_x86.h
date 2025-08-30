#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

#include <devices/cpu/cpu_defs.h>

struct generic_driver_tree_node_t;


bool cpu_scan_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mps_drv_node,
    table_callback_t callback,
    void* ctx
);
struct generic_driver_tree_node_t* cpu_create_mp_table_node(
    struct generic_driver_tree_node_t* cpu_self
);

void cpu_remove_mp_table_node(
    struct generic_driver_tree_node_t* cpu_self, 
    struct generic_driver_tree_node_t* mps_driver_node 
);