#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

struct generic_driver_tree_node_t;
struct acpi_sdt_hdr_t;
typedef struct acpi_sdt_hdr_t acpi_sdt_hdr_t;

struct acpi_sdt_list_t;
typedef struct acpi_sdt_list_t acpi_sdt_list_t;

bool acpi_table_is_sane(acpi_sdt_hdr_t* hdr);
bool acpi_invalidate_table(
    struct generic_driver_tree_node_t* self, 
    struct generic_driver_tree_node_t* table_drv_node,
    acpi_sdt_list_t* table_list_node
);
bool acpi_invalidate_all_tables(
    struct generic_driver_tree_node_t* self, 
    struct generic_driver_tree_node_t* table_drv_node
);

struct generic_driver_tree_node_t* acpi_find_table(
    struct generic_driver_tree_node_t* self, 
    char* signature
);