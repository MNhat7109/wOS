#pragma once
#include "../../stdint.h"
#include "../driver.h"

#define PIC_DRV_CMD_RECEIVE_IRR 0
#define PIC_DRV_CMD_RECEIVE_ISR 1

#define PIC_DRV_CMD_SEND_EOI    2
#define PIC_DRV_CMD_SEND_UNMASK 3
#define PIC_DRV_CMD_SEND_MASK   4
#define PIC_DRV_CMD_SEND_CDW1   5 // config DWORDs
#define PIC_DRV_CMD_SEND_CDW2   6
#define PIC_DRV_CMD_SEND_MASK_VAL   7
#define PIC_DRV_CMD_IDLE        0xFF

const generic_driver_t* i8259_get_driver();