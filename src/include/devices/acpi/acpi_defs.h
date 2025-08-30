#pragma once
#include <libk/stdint.h>

#define SDP_TYPE_ACPI10 1
#define SDP_TYPE_ACPI20 2

typedef struct acpi_param_t
{
    void* sys_desc_ptr;
    usize _reserved[3];
} acpi_param_t;

typedef struct system_desc_ptr_t
{
    int type;
    usize address;
} __attribute__((packed)) system_desc_ptr_t;

typedef struct acpi_sdt_hdr_t
{
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} __attribute__((packed)) acpi_sdt_hdr_t;

struct acpi_sdt_list_t;
typedef struct acpi_sdt_list_t acpi_sdt_list_t;
typedef struct acpi_sdt_list_t
{
    acpi_sdt_hdr_t* hdr;
    acpi_sdt_list_t* next;
} acpi_sdt_list_t;