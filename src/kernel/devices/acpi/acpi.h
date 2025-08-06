#pragma once
#include "../driver.h"
#include <stdbool.h>

struct system_desc_ptr_t;
typedef struct system_desc_ptr_t system_desc_ptr_t;

struct acpi_sdt_hdr_t;
typedef struct acpi_sdt_hdr_t acpi_sdt_hdr_t;

struct acpi_driver_t
{
    struct generic_driver_t driver_hdr;
    system_desc_ptr_t* rxsdp;
    u8 __ptr_size;
    u32 __sdt_base;
    u32 __sdt_count;
    acpi_sdt_hdr_t* (*get_table)(struct acpi_driver_t*, char*);
} __attribute__((packed));

const struct generic_driver_t* acpi_get_driver();