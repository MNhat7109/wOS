#pragma once
#include "../stdint.h"

typedef struct
{
    int type;
    u32 address;
} __attribute__((packed)) system_desc_ptr_t;

typedef struct
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

void ACPI_init(system_desc_ptr_t* sdp);
acpi_sdt_hdr_t* ACPI_find_table(char* signature);