#include "i8254.h"
#include "../../stdint.h"
#include "../../io/io.h"

#define PIT_C0_DATA_PORT 0x40
#define PIT_C1_DATA_PORT 0x41
#define PIT_C2_DATA_PORT 0x42
#define PIT_CMD_PORT 0x43

void i8254_config(u16 reload_value)
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

pit_driver_t i8254_driver;
const pit_driver_t* i8254_get_driver()
{
    i8254_driver.name = "8254 PIT";
    i8254_driver.probe = &i8254_probe;
    i8254_driver.read_current_counter = &i8254_read_back;
    i8254_driver.config = &i8254_config;
    return &i8254_driver;
}