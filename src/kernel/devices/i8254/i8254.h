#pragma once
#include "../driver.h"

#define PIT_DRV_CMD_SEND_CDW1     0
#define PIT_DRV_CMD_RVAL          1
#define PIT_DRV_CMD_RECEIVE_CNTER 2
#define PIT_DRV_CMD_IDLE 0xFF

const generic_driver_t* i8254_get_driver();