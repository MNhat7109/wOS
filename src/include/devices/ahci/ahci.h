#pragma once
#include <devices/driver.h>
#include <stdbool.h>
#include <libk/stdint.h>

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;

struct ahci_dev_t;
typedef struct ahci_dev_t ahci_dev_t;

typedef void (*ahci_export_cb_t)(ahci_controller_t* ctl, u32 ctl_count);

struct ahci_driver_t
{
    struct generic_driver_t driver_hdr;
    void (*export_dev)(struct generic_driver_t*, ahci_export_cb_t);
    bool (*ioctl)(struct generic_driver_t*, ahci_dev_t*, int, void*);
} __attribute__((packed));

const struct generic_driver_t* ahci_get_driver();