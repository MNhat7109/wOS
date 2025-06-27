#include "ahci_info.h"
#include "ahci_drv.h"
#include "../driver.h"
#include "../../stdint.h"
#include <stdbool.h>

void ahci_write(int pool, u32 value)
{
    ahci_io_pack.pool_value = value;
    switch (pool)
    {
    case DRIVER_CMD:
        switch (ahci_io_pack.pool_value)
        {
            case AHCI_DRV_CMD_RECEIVE_PORT_CNT:
                ahci_io_pack.cmd_sig = ahci_io_pack.pool_value;
                ahci_io_pack.pool_value = 0;
                ahci_io_pack.receive = true;
                break;
        }
        break;
    default:
        ahci_io_pack.send = false;
        switch (ahci_io_pack.cmd_sig)
        {
        default:
            break;
        }
        ahci_io_pack.cmd_sig = AHCI_DRV_CMD_IDLE;
        break;
    }
}