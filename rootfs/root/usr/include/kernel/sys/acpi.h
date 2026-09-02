#pragma once
#include <stdint.h>

typedef struct system_desc_ptr_t system_desc_ptr_t;


typedef struct system_desc_ptr_t
{
    int type;
    u32 address;
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

int acpi_init(system_desc_ptr_t* sdp);
acpi_sdt_hdr_t* acpi_get_table(char* signature);
// void acpi_invalidate_table(acpi_sdt_hdr_t* tbl);