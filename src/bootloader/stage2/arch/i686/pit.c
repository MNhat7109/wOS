#include "../../drivers/pit.h"
#include "../../drivers/pic.h"
#include "io.h"
#include "irq.h"

// u32 timer_frac=0, timer_ms=0;
volatile u32 countdown=0;

void timer_handler(registers_t* regs)
{
    if (countdown!=0) countdown--;
    PIC_send_eoi(0);
}

void PIT_init(u32 reload_value)
{
    //kprintf("%u.%u\n", irq0_ms, irq0_frac);
    i686_disable_interrupt();

    outb(0x40, (u8)(reload_value&0xFF));
    outb(0x40, (u8)((reload_value>>8)&0xFF));

    i686_enable_interrupt();

    i686_irq_register_handler(0, timer_handler);
}

void PIT_set_countdown_value(u32 value)
{
    countdown=value;
}

void PIT_start_countdown()
{
    while (countdown);
}