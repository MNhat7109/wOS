#pragma once

struct generic_driver_tree_node_t;
struct cpu_core_additional_param_t;
typedef struct cpu_core_additional_param_t cpu_core_additional_param_t;

void ioapic_init(
    struct generic_driver_tree_node_t* cpu_self, 
    cpu_core_additional_param_t* required
);

void ioapic_disable(struct generic_driver_tree_node_t* cpu_self);