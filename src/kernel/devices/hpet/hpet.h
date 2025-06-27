#pragma once
#include "../driver.h"

#define HPET_DRV_CMD_SEND_MMIO 0x00
#define HPET_DRV_CMD_SEND_MMIO_OFFSET 0x01
#define HPET_DRV_CMD_SEND_MMIO_VALUE_LO 0x02
#define HPET_DRV_CMD_SEND_MMIO_VALUE_HI 0x03
#define HPET_DRV_CMD_RECIEVE_MMIO_LO 0x10
#define HPET_DRV_CMD_RECIEVE_MMIO_HI 0x11
#define HPET_DRV_CMD_IDLE 0xFF

const generic_driver_t* hpet_get_driver();