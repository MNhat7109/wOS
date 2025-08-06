#pragma once

struct acpi_driver_t;
struct acpi_sdt_hdr_t;
typedef struct acpi_sdt_hdr_t acpi_sdt_hdr_t;

acpi_sdt_hdr_t* acpi_find_table(struct acpi_driver_t* self, char* signature);