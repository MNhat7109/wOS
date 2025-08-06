#pragma once
#include "../../stdint.h"

#define AHCI_CONFIG_MAX_CTL 4
#define AHCI_CONFIG_MAX_PORT 32
#define AHCI_CONFIG_MAX_CMD_SLOT 32
#define AHCI_CONFIG_MAX_ABAR_PAGE 2

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;
struct pci_driver_t;
struct generic_driver_t;

extern struct ahci_shared_t
{
    ahci_controller_t* controllers;
    u32 ctl_count;
    struct pci_driver_t* pci_dev;
} ahci_shared;

