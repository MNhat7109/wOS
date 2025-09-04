#pragma once

struct generic_driver_tree_node_t;
struct x86_cpu_global_additional_param_t;
typedef struct x86_cpu_global_additional_param_t x86_cpu_global_additional_param_t;

void ioapic_init(
    struct generic_driver_tree_node_t* cpu_self, 
    x86_cpu_global_additional_param_t* required
);

void ioapic_disable(struct generic_driver_tree_node_t* cpu_self);