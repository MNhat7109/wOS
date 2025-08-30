#pragma once

struct generic_driver_tree_node_t;

void lapic_init(
    struct generic_driver_tree_node_t* cpu_core_self
);
void lapic_global_init(
    struct generic_driver_tree_node_t* cpu_self
);

void lapic_disable(
    struct generic_driver_tree_node_t* cpu_core_self
);