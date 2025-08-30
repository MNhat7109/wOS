#pragma once
#include <devices/driver.h>
#include <libk/stdint.h>
#include <stdbool.h>

struct cpu_ipi_request_t;
struct cpu_interrupt_ops_t;

struct cpu_driver_ops_t
{
    struct generic_driver_ops_t ops_hdr;
    int (*send_ipi)(
        struct generic_driver_tree_node_t* self,
        const struct cpu_ipi_request_t* req
    );
    int (*broadcast_ipi)(
        struct generic_driver_tree_node_t* self,
        const struct cpu_ipi_request_t* req
    );
    void (*sync_barrier)(
        struct generic_driver_tree_node_t* self
    );
    
    const struct cpu_interrupt_ops_t* global_interrupt_ops;
};

const struct generic_driver_ops_t* cpu_get_driver_ops();