#include "hpet.h"
#include "../../stdint.h"
#include "../../stdio.h"
#include <stdbool.h>
#include "../../paging/paging.h"
#include "../../acpi/acpi.h"

typedef struct
{
    u8 address_space_id;
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 _reserved;
    u64 base;
} __attribute__((packed)) hpet_address_t;

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u8 revision_id;
    u8 comparator_count : 5;
    u8 counter_size : 1;
    u8 _reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendor_id;
    hpet_address_t hpet_address;
    u8 hpet_num;
    u16 min_tick;
    u8 page_protection;
} __attribute__((packed)) hpet_t;

u32 hpet_phys;
volatile u64* hpet_base;
hpet_t* hpet;
u32 hpet_mmio_offset;
u64 hpet_mmio_value;
generic_driver_io_t hpet_io_pack =
{
    .pool_value = 0,
    .cmd_sig = HPET_DRV_CMD_IDLE,
    .receive=false,
    .send=false
};

void hpet_mmio_write(u32 offset, u64 value)
{
    *(u64*)(hpet_phys+offset) = value;
}

u64 hpet_mmio_read(u32 offset)
{
    return *(u64*)(hpet_phys+offset);
}

bool hpet_probe()
{
    hpet = (hpet_t*)ACPI_find_table("HPET");
    return hpet;
}

void hpet_read_capabilities()
{
    u64 hpet_cap = hpet_mmio_read(0);
    kprintf("HPET Capability: Revision ID: %u\n"
        , (hpet_cap>>0) & 0xFF);
    kprintf("HPET Capability: Timer Count: %u\n"
        , ((hpet_cap>>8) & 0x1F)-1);
    kprintf("HPET Capability: 64-bit Mode Counter: %s\n"
        , (const char*[]){"no", "yes"}[(hpet_cap>>13) & 0x1]);
    kprintf("HPET Capability: Legacy Replacement Support: %s\n"
        , (const char*[]){"no", "yes"}[(hpet_cap>>15) & 0x1]);
    kprintf("HPET Capability: Vendor ID: %u\n"
        , (hpet_cap>>16) & 0xFFFF);
    kprintf("HPET Capability: Femtoseconds per Tick: %u\n"
        , (hpet_cap>>32) & 0xFFFFFFFF);
}

void hpet_write(int pool, u32 value)
{
    hpet_io_pack.pool_value = value;
    switch (pool)
    {
    case DRIVER_CMD:
        hpet_io_pack.cmd_sig = hpet_io_pack.pool_value;
        switch (hpet_io_pack.pool_value)
        {
            case HPET_DRV_CMD_RECIEVE_MMIO_HI:
            case HPET_DRV_CMD_RECIEVE_MMIO_LO:
            hpet_io_pack.receive = true;
            break;
        case HPET_DRV_CMD_SEND_MMIO:
        case HPET_DRV_CMD_SEND_MMIO_OFFSET:
        case HPET_DRV_CMD_SEND_MMIO_VALUE_LO:
        case HPET_DRV_CMD_SEND_MMIO_VALUE_HI:
        hpet_io_pack.send = true;
            break;
        }
        hpet_io_pack.pool_value = 0;
        break;
    default:
        switch (hpet_io_pack.cmd_sig)
        {
            case HPET_DRV_CMD_SEND_MMIO_OFFSET:
            hpet_mmio_offset = hpet_io_pack.pool_value;
            break;
            case HPET_DRV_CMD_SEND_MMIO_VALUE_LO:
            hpet_mmio_value |= hpet_io_pack.pool_value;
            break;
            case HPET_DRV_CMD_SEND_MMIO_VALUE_HI:
            hpet_mmio_value |= ((u64)hpet_io_pack.pool_value<<32);
            break;
            case HPET_DRV_CMD_SEND_MMIO:
            hpet_mmio_write(hpet_mmio_offset, hpet_mmio_value);
            break;
        }
        hpet_io_pack.send=false;
        hpet_io_pack.pool_value=0;
        hpet_io_pack.cmd_sig = HPET_DRV_CMD_IDLE;
        break;
    }
}

u32 hpet_read()
{
    if (!hpet_io_pack.receive) return 0;
    hpet_io_pack.pool_value = 0;
    hpet_io_pack.receive = false;
    u32 ret_val =0;
    switch (hpet_io_pack.cmd_sig)
    {
    case HPET_DRV_CMD_RECIEVE_MMIO_HI:
        ret_val = hpet_mmio_read(hpet_mmio_offset)>>32;
        break;
        case HPET_DRV_CMD_RECIEVE_MMIO_LO:
        ret_val = hpet_mmio_read(hpet_mmio_offset);
        break;
        default: break;
    }
    hpet_io_pack.cmd_sig = HPET_DRV_CMD_IDLE;
    return ret_val;
}

void hpet_config()
{
    hpet_phys = hpet->hpet_address.base;
    kprintf("HPET: Base=0x%x\n", hpet_phys);
    page_manager_map_memory(hpet_phys, hpet_phys);
    hpet_base = (u64*)hpet_phys;
    kprintf("HPET: Reading capabilities\n");
    hpet_read_capabilities();

    hpet_mmio_write(0xF0, 0); // Reset main counter
    u64 conf = hpet_mmio_read(0x10);
    conf|=1;
    hpet_mmio_write(0x10, conf); // Enable HPET
}

generic_driver_t hpet_driver = 
{ 
    .name = "HPET",
    .config = &hpet_config,
    .probe = &hpet_probe,
    .read = &hpet_read,
    .write = &hpet_write,
};

const generic_driver_t* hpet_get_driver()
{
    return &hpet_driver;
}