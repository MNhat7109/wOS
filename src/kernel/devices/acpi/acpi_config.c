#include "acpi_config.h"
#include "acpi.h"
#include "acpi_defs.h"
#include "../../paging/paging.h"

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

void acpi_config(struct generic_driver_t* driver)
{
    struct acpi_driver_t* acpi_self = (struct acpi_driver_t*)driver;
    u32 rxsdt;

    page_manager_map_memory((u32)acpi_self->rxsdp, (u32)acpi_self->rxsdp);
    page_manager_map_memory(acpi_self->rxsdp->address, acpi_self->rxsdp->address);

    if (acpi_self->rxsdp->type == 1)
    {
        rxsdt = ((rsdp_t*)acpi_self->rxsdp->address)->rsdt_addr;
        acpi_self->__ptr_size = 4;
    }
    else
    {
        rxsdt = ((xsdp_t*)acpi_self->rxsdp->address)->xsdt_addr;
        acpi_self->__ptr_size = 8;
    }

    page_manager_map_memory(rxsdt, rxsdt);
    acpi_sdt_hdr_t* rxsdt_base = (acpi_sdt_hdr_t*)rxsdt;
    acpi_self->__sdt_count = (rxsdt_base->length - sizeof(acpi_sdt_hdr_t)) / acpi_self->__ptr_size;
    acpi_self->__sdt_base = (rxsdt+sizeof(acpi_sdt_hdr_t));

    // Map from base to base+length
    u32 num_pages = page_convert_from_bytes((rxsdt_base->length - sizeof(acpi_sdt_hdr_t)));
    u32 current_page = acpi_self->__sdt_base;
    for (u32 i=0;i<num_pages;i++)
    {
        page_manager_map_memory(current_page, current_page);
        current_page +=0x1000;
    }
}

void acpi_disable(struct generic_driver_t* driver)
{
    struct acpi_driver_t* acpi_self = (struct acpi_driver_t*)driver;

    u32 num_pages = page_convert_from_bytes(acpi_self->__sdt_count*acpi_self->__ptr_size);
    u32 current_page = acpi_self->__sdt_base;
    bool ok;

    for (u32 i=0;i<num_pages;i++)
    {
        ok = page_manager_unmap_memory(current_page);
        if (!ok)
        {
            driver_log_state(driver, DRIVER_LOG_ERROR,
                "Error unmapping the RSDT/XSDT table");
            return;
        }
        current_page+=0x1000;
    }

    u32 rxsdt_hdr = acpi_self->__sdt_base-sizeof(acpi_sdt_hdr_t);
    ok = page_manager_unmap_memory(rxsdt_hdr);
    if (!ok)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, 
            "Error unmapping the RSDT/XSDT header");
        return;
    }

    ok = page_manager_unmap_memory(acpi_self->rxsdp->address);
    if (!ok)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, 
            "Error unmapping the bootloader RSDP/XSDP base address");
        return;
    }
    
    ok = page_manager_unmap_memory((u32)acpi_self->rxsdp);
    if (!ok)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, 
            "Error unmapping the bootloader RSDP/XSDP header");
        return;
    }
}