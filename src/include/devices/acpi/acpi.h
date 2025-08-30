#pragma once
#include <devices/driver.h>
#include <stdbool.h>

struct system_desc_ptr_t;
typedef struct system_desc_ptr_t system_desc_ptr_t;

struct acpi_sdt_hdr_t;
typedef struct acpi_sdt_hdr_t acpi_sdt_hdr_t;

struct acpi_sdt_list_t;
typedef struct acpi_sdt_list_t acpi_sdt_list_t;

struct acpi_driver_ops_t
{
    struct generic_driver_ops_t ops_hdr;

    struct generic_driver_tree_node_t* (*get_table)(
        struct generic_driver_tree_node_t* node, 
        char* signature
    );
    bool (*invalidate_table)(
        struct generic_driver_tree_node_t* self, 
        struct generic_driver_tree_node_t* table_drv_node,
        acpi_sdt_list_t* table_list_node
    );
    bool (*invalidate_all_tables)(
        struct generic_driver_tree_node_t* self, 
        struct generic_driver_tree_node_t* table_drv_node
    );
} __attribute__((packed));

const struct generic_driver_ops_t* acpi_get_driver_ops();