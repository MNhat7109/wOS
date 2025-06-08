#include "acpi.h"
#include "../string/string.h"
#include "../stdio.h"

typedef struct
{
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
} __attribute__((packed)) rsdp_t;

typedef struct
{
    rsdp_t rsdp;

    u32 length;
    u64 xsdt_addr;
    u8 ext_checksum;
    u8 reserved[3];
} __attribute__((packed)) xsdp_t;

struct
{
    u32 sdt_address;
    u32 sdt_entries;
    u8* pointer_to_sdts;
    u32 offset;
} __attribute__((packed)) acpi_data_pack;

void ACPI_init(system_desc_ptr_t* sdp)
{
    if (sdp->type == 1)
    {
        acpi_data_pack.sdt_address = ((rsdp_t*)sdp->address)->rsdt_addr; 
        acpi_data_pack.offset = 4;
    }
    else 
    {
        acpi_data_pack.sdt_address = ((xsdp_t*)sdp->address)->xsdt_addr;
        acpi_data_pack.offset = 8;
    }
    acpi_data_pack.sdt_entries = 
    (((acpi_sdt_hdr_t*)acpi_data_pack.sdt_address)->length - sizeof(acpi_sdt_hdr_t))
    /acpi_data_pack.offset;
    acpi_data_pack.pointer_to_sdts = (u8*)(acpi_data_pack.sdt_address+sizeof(acpi_sdt_hdr_t));
}

acpi_sdt_hdr_t* ACPI_find_table(char* signature)
{
    for (u32 i=0;i<acpi_data_pack.sdt_entries;i++)
    {
        u64 sdt_ptr;
        memcpy(&sdt_ptr, acpi_data_pack.pointer_to_sdts+i*acpi_data_pack.offset, acpi_data_pack.offset);
        acpi_sdt_hdr_t* new_hdr = (acpi_sdt_hdr_t*)sdt_ptr;
        if (memcmp(new_hdr->signature, signature, 4) == 0)
        {
            kprintf("Found header with signature: %s, base: 0x%x\n", signature, new_hdr);
            return new_hdr;
        }
    }
    return NULL;
}