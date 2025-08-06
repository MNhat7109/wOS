#include "irq_pic.h"
#include "irq_utils.h"
#include "irq_defs.h"

#include "../../devices/i8259/i8259.h"
#include "../../stdio.h"

struct pic_driver_t* pic_dev;
#define MAX_HANDLED_IRQ 16
#define PIC_REMAP_OFFSET 0x20

extern u8 handler_offset;

bool irq_set_up_pic()
{
    pic_dev = (struct pic_driver_t*)driver_get("8259 PIC");
    if (!pic_dev)
    {
        kprintf("Interrupt: 8259 PIC driver not found\n");
        return false;
    }

    pic_dev->offset_pic1 = PIC_REMAP_OFFSET;
    pic_dev->offset_pic2 = PIC_REMAP_OFFSET+8;
    if (!driver_run((struct generic_driver_t*)pic_dev))
    {
        kprintf("Interrupt: 8259 PIC driver failed\n");
        driver_unload("8259 PIC");
        return false;
    }

    return true;
}

void irq_pic_setup_int(int interrupt, gsi_handler_t handler, void* ctx)
{
    if (interrupt < 0 || interrupt > MAX_HANDLED_IRQ) return;
    
    irq_load_handler(interrupt+handler_offset, handler, ctx);
    pic_dev->unmask(pic_dev, interrupt);
}

void irq_pic_disable_int(int interrupt)
{
    if (interrupt < 0 || interrupt > MAX_HANDLED_IRQ) return;

    irq_unload_handler(interrupt+handler_offset);
    pic_dev->mask(pic_dev, interrupt);
}

void irq_pic_end(int interrupt)
{
    pic_dev->send_eoi(pic_dev, interrupt);
}

u8 irq_pic_get_vector_cnt()
{
    return MAX_HANDLED_IRQ;
}

u8 irq_pic_get_vector_offset()
{
    return PIC_REMAP_OFFSET;
}