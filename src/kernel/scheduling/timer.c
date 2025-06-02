#include "timer.h"
#include "../devices/i8254/i8254.h"
#include "../hal/interrupt/irq.h"
#include "../x86/x86.h"
#include "../stdio.h"

#define UINT32_MAX 0xFFFFFFFFU

volatile u32 timer_ms = 0;
volatile u32 timer_frac_ms = 0;
volatile u32 tick_increment = UINT32_MAX / 1000;

const pit_driver_t* pit_driver = NULL;

void timer_tick()
{
    // timer_frac_ms+=tick_increment;
    // if (timer_frac_ms<tick_increment) 
    // {
    //     timer_ms++;
    // }
    timer_ms++;
}

void sleep(u32 ms)
{
    u32 start_ms=timer_ms;
    while (timer_ms < start_ms+ms) _x86_halt();
}

// void nsleep(u32 ns)
// {
//     u32 start_ms = timer_ms;
//     u32 start_frac=timer_frac_ms;
//     u32 frac = (ns*UINT32_MAX)/1000000;
//     start_frac+=frac;
//     if (start_frac < frac) start_ms++; 
//     while (timer_ms<start_ms||(timer_ms==start_ms&&timer_frac_ms<start_frac))
//         _x86_halt();
// }

void timer_int_handler(registers_t* regs)
{
    timer_tick();
    pic_driver->send_eoi(0);
}

void timer_init()
{
    pit_driver_t* pit_drv[] = {i8254_get_driver(), };
    u32 drv_cnt = sizeof(pit_drv)/sizeof(pit_drv[0]);
    for (u32 i=0;i<drv_cnt;i++)
    {
        if (pit_drv[i]->probe())
        {
            pit_driver = pit_drv[i];
            break;
        }
    }
    if (!pit_driver) return;

    _x86_disable_interrupt();
    pit_driver->config(1193); // This will give us closer (if not exact) 1 ms interrupts
    _x86_enable_interrupt();
    
    IRQ_reg_handler(0, timer_int_handler);
    pic_driver->unmask(0);
}