#pragma once
#include "../../stdint.h"
#include <stdbool.h>
#include "../driver.h"
#include "../mmio.h"
#include "ahci_ports.h"

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

struct ahci_driver_t
{
    struct generic_driver_t driver_hdr;
    hba_memory_t* __abar;
    u8 __irq;
    u32 __bar5;
    ahci_ports_t ports;
    int (*read)(struct ahci_driver_t*, ahci_device_entry_t*
        , u64, u16, void*);
    int (*write)(struct ahci_driver_t*, ahci_device_entry_t*
        , u64, u16, void*);
} __attribute__((packed));

const struct generic_driver_t* ahci_get_driver();