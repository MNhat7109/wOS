#pragma once
#include "../driver.h"

#define AHCI_DRV_CMD_RECEIVE_PORT_CNT 0
#define AHCI_DRV_CMD_IDLE 0xFF

const generic_driver_t* ahci_get_driver();