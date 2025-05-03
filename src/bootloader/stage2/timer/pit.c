#include "pit.h"
#include "../io/io.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/pic.h"
#include "../stdio.h"
#include <stdbool.h>

u32 timer_frac=0, timer_ms=0;
u32 irq0_frac, irq0_ms;
volatile u32 countdown=0;

void tick();

void timer_handler(registers_t* regs)
{
    if (countdown!=0) countdown--;
    tick();
    PIC_send_eoi(0);
}

void sleep(u32 ms)
{
    countdown=ms;
    while (countdown) _x86_halt();
}

void tick()
{
    if ((u64)timer_frac+irq0_frac > 0xFFFFFFFF)
    {
        timer_ms=timer_ms+irq0_ms+1;
    }
    else timer_ms+=irq0_ms;
    timer_frac+=irq0_frac;
}
void PIT_init(u32 reload_value)
{
    irq0_frac = (0xDBB3A062*reload_value)>>10;
    irq0_ms = ((0xDBB3A062*reload_value)&0x3FF)>>10;
    //kprintf("%u.%u\n", irq0_ms, irq0_frac);
    _x86_disable_interrupt();

    outb(0x40, (u8)(reload_value&0xFF));
    iowait();

    outb(0x40, (u8)((reload_value>>8)&0xFF));

    _x86_enable_interrupt();

    IRQ_reg_handler(0, timer_handler);
}