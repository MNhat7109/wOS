#include "irq.h"
#include "../../devices/i8259/pic.h"
#include "../../devices/i8259/i8259.h"
#include "../../stdint.h"
#include "../../stdio.h"
#include "../../x86/x86.h"

#define PIC_REMAP_OFFSET 0x20

irq_handler_t irq_handler_table[16];
const pic_driver_t* driver = NULL;

void irq_handler(registers_t* regs)
{
    int irq = regs->vector - PIC_REMAP_OFFSET;

    if (irq_handler_table[irq]) irq_handler_table[irq](regs);
    else
    {
        kprintf("PIC: IRQ no. %d unhandled!", irq);
    }

    driver->send_eoi(irq);
}

void IRQ_init()
{
    const pic_driver_t* pic_drv[] = {i8259_get_driver(),};

    u32 drv_cnt = sizeof(pic_drv)/sizeof(pic_drv[0]);
    for (int i=0;i<drv_cnt;i++)
        if (pic_drv[i]->probe())
        {
            driver = pic_drv[i];
        }

    if (!driver)
    {
        kprintf("PIC: No PICs found!\n");
        return;
    }
    driver->config(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET+8);
    for (int i=0;i<16;i++)
    {
        ISR_reg_handler(PIC_REMAP_OFFSET+i, irq_handler);
    }
    _x86_enable_interrupt();
}

void IRQ_reg_handler(int irq, irq_handler_t handler)
{
    irq_handler_table[irq] = handler;
}