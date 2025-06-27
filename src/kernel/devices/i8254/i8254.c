#include "i8254.h"
#include "../../stdint.h"
#include "../../io/io.h"

#define PIT_C0_DATA_PORT 0x40
#define PIT_C1_DATA_PORT 0x41
#define PIT_C2_DATA_PORT 0x42
#define PIT_CMD_PORT 0x43

generic_driver_io_t pit_io_util_pack = 
{
    .pool_value = 0,
    .cmd_sig = PIT_DRV_CMD_IDLE,
    .receive=false,
    .send=false
};
u16 reload_value = 0;

void i8254_config()
{
    // Write to channel 0. It is the most frequently used timer
    // for interrupts
    outb(PIT_C0_DATA_PORT, (reload_value&0xFF));
    outb(PIT_C0_DATA_PORT, ((reload_value>>8)&0xFF));    
}

u16 i8254_read_back()
{
    outb(PIT_CMD_PORT, 0xC2);

    u8 lo = inb(PIT_C0_DATA_PORT);
    u8 hi = inb(PIT_C0_DATA_PORT);

    u16 current_count = lo | (hi << 8);
    return current_count;
}

bool i8254_probe()
{
    u16 old_counter = i8254_read_back();

    u32 delay = 1000;
    while (delay--);

    u16 new_counter = i8254_read_back();
    return new_counter < old_counter;
}

u32 i8254_read()
{
    if (!pit_io_util_pack.receive) return 0;
    pit_io_util_pack.pool_value=0;
    pit_io_util_pack.receive=false;
    u32 ret_val = 0;
    switch (pit_io_util_pack.cmd_sig)
    {
    case PIT_DRV_CMD_RVAL:
        ret_val = reload_value;
        break;
    case PIT_DRV_CMD_RECEIVE_CNTER:
        ret_val = i8254_read_back();
        break;
    default:
        break;
    }
    pit_io_util_pack.cmd_sig = PIT_DRV_CMD_IDLE;
    return ret_val;
}

void i8254_write(int pool, u32 value)
{
    pit_io_util_pack.pool_value = value;
    switch (pool)
    {
    case DRIVER_CMD:
        switch (pit_io_util_pack.pool_value)
        {
        case PIT_DRV_CMD_RVAL:
            pit_io_util_pack.receive=true;
        case PIT_DRV_CMD_SEND_CDW1:
            pit_io_util_pack.cmd_sig = pit_io_util_pack.pool_value;
            pit_io_util_pack.pool_value = 0;
            pit_io_util_pack.send = true;
            break;
        case PIT_DRV_CMD_RECEIVE_CNTER:
            pit_io_util_pack.cmd_sig = pit_io_util_pack.pool_value;
            pit_io_util_pack.pool_value = 0;
            pit_io_util_pack.receive= true;
            break;
        }
        break;
    default:
        pit_io_util_pack.send=false;
        switch (pit_io_util_pack.cmd_sig)
        {
        case PIT_DRV_CMD_RVAL:
        case PIT_DRV_CMD_SEND_CDW1:
            reload_value=pit_io_util_pack.pool_value;
            pit_io_util_pack.pool_value=0;
            break;
        default:
            break;
        }
        pit_io_util_pack.cmd_sig=PIT_DRV_CMD_IDLE;
        break;
    }
}

void i8254_disable()
{
}

generic_driver_t i8254_driver = {
    .name = "8254 PIT",
    .config = &i8254_config,
    .probe = &i8254_probe,
    .read = &i8254_read,
    .write = &i8254_write,
    .disable = &i8254_disable
};
const generic_driver_t* i8254_get_driver()
{
    return &i8254_driver;
}