#include "rsdp.h"
#include "../stdio.h"
#include "../string/string.h"

#define MEMORY_EBDA_BASE 0x40E
#define MEMORY_EBDA_END 0x9FFFF
#define MEMORY_BIOS_RESERVED_START 0xE0000
#define MEMORY_BIOS_RESERVED_END   0xFFFFF

system_desc_ptr_t sys_struct;

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


int RSDP_detect_ACPI(u32 rsdp)
{
    rsdp_t* ptr = (rsdp_t*)rsdp;
    return (ptr->revision == 2)? 2: 1;
}

bool RSDP_checksum_check(u32 rsdp)
{
    u32 sum=0;
    u8* ptr = (u8*)rsdp;
    u32 len = sizeof(rsdp_t); // 20
    for (u32 i=0;i<len;i++) sum+=ptr[i];
    u8 to_check = sum;
    return to_check==0;
}

bool RSDP_ext_checksum_check(u32 xsdp)
{
    u32 sum=0;
    u8* ptr = (u8*)xsdp;
    u32 len = sizeof(xsdp_t);
    for (u32 i=0;i<len;i++) sum+=ptr[i];
    u8 to_check = sum;
    return to_check==0;
}

bool RSDP_scan(system_desc_ptr_t** address)
{
    const char* rsdp_str = "RSD PTR "; 
    // Find RSDP in EBDA first
    u32 ebda_addr = *(u16*)MEMORY_EBDA_BASE << 4;
    u32 ebda_size = MEMORY_EBDA_END-ebda_addr+1;
    u32 desired_addr;

    // Second option
    u32 area_size = MEMORY_BIOS_RESERVED_END-MEMORY_BIOS_RESERVED_START+1;
    
    // Option 1: EBDA
    for (u32 i=0;i<ebda_size;i+=16)
    {
        u32 addr = ebda_addr+i;
        if (memcmp((const void*)addr, rsdp_str, 8) == 0)
        {
            int acpi_ver = RSDP_detect_ACPI(addr);
            bool check = (acpi_ver==2)?RSDP_ext_checksum_check(addr)
            :RSDP_checksum_check(addr);
            if (!check) continue;
            // Found RSDP
            sys_struct.address = addr;
            sys_struct.type=acpi_ver;
            kprintf("ACPI version: %d, RSDP/XSDP: 0x%x\n",
            sys_struct.type, sys_struct.address);
            *address = &sys_struct;
            return true;
        }
    }

    // Option 2: Extended BIOS area
    for (u32 i=0;i<area_size;i+=16)
    {
        u32 addr = MEMORY_BIOS_RESERVED_START+i;
        if (memcmp((const void*)addr, rsdp_str, 8) == 0)
        {
            int acpi_ver = RSDP_detect_ACPI(addr);
            bool check = (acpi_ver==2)?RSDP_ext_checksum_check(addr)
            :RSDP_checksum_check(addr);
            if (!check) continue;
            // Found RSDP
            sys_struct.address = addr;
            sys_struct.type=acpi_ver;
            kprintf("ACPI version: %d, RSDP/XSDT: 0x%x\n",
            sys_struct.type, sys_struct.address);
            *address = &sys_struct;
            return true;
        }
    }
    return false;
}
