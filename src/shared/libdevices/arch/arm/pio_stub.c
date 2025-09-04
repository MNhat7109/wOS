#include <devices/pio.h>

int pio_acquire(struct pio_info_t* self, u16 port)
{
    return -PIO_ERR_UNSUPPORTED;
}

int pio_release(struct pio_info_t* self)
{
    return -PIO_ERR_UNSUPPORTED;
}