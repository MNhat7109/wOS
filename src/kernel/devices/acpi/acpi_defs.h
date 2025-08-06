#pragma once
#include "../../stdint.h"

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