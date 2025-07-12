#include "i8254.h"
#include "../../stdint.h"


void i8254_config(struct generic_driver_t* driver)
{
    struct pit_driver_t* pit = (struct pit_driver_t*)driver;
    // Write to channel 0. It is the most frequently used timer
    // for interrupts
    pit->pio_utils.writeb(PIT_C0_DATA_PORT, (pit->reload_value&0xFF));
    pit->pio_utils.writeb(PIT_C0_DATA_PORT, ((pit->reload_value>>8)&0xFF));    
}

u16 i8254_read_back(struct pit_driver_t* pit)
{
    pit->pio_utils.writeb(PIT_CMD_PORT, 0xC2);

    u8 lo = pit->pio_utils.readb(PIT_C0_DATA_PORT);
    u8 hi = pit->pio_utils.readb(PIT_C0_DATA_PORT);

    u16 current_count = lo | (hi << 8);
    return current_count;
}

bool i8254_probe(struct generic_driver_t* driver)
{
    struct pit_driver_t* pit = (struct pit_driver_t*)driver;
    u16 old_counter = i8254_read_back(pit);

    u32 delay = 1000;
    while (delay--);

    u16 new_counter = i8254_read_back(pit);
    return new_counter < old_counter;
}

void i8254_disable(struct generic_driver_t* driver)
{
    struct pit_driver_t* pit = (struct pit_driver_t*)driver;
    pit->pio_utils.writeb(PIT_CMD_PORT, 0b00011010);
    pit->reload_value = 1;
    i8254_config(driver);
}

struct pit_driver_t i8254_driver = {
    .driver_hdr = {
        .name="i8254 PIT", 
        .config=&i8254_config,
        .disable=&i8254_disable,
        .probe=&i8254_probe
    },
    .read_counter = &i8254_read_back,
    .reload_value = 0
};

const struct generic_driver_t* i8254_get_driver()
{
    i8254_driver.pio_utils = pio_load_defaults();
    return (struct generic_driver_t*)&i8254_driver;
}