#include "pit.h"
#include "../io/io.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/pic.h"
#include "../stdio.h"
#include <stdbool.h>

u32 timer_frac=0, timer_ms=0;
volatile u32 countdown=0;

void timer_handler(registers_t* regs)
{
    if (countdown!=0) countdown--;
    PIC_send_eoi(0);
}

void sleep(u32 ms)
{
    countdown=ms;
    while (countdown) //__asm__ volatile("hlt");
        _x86_halt();
}

void PIT_init(u32 reload_value)
{
    //kprintf("%u.%u\n", irq0_ms, irq0_frac);
    _x86_disable_interrupt();

    outb(0x40, (u8)(reload_value&0xFF));
    outb(0x40, (u8)((reload_value>>8)&0xFF));

    _x86_enable_interrupt();

    IRQ_reg_handler(0, timer_handler);
}