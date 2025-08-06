#include "irq.h"
#include "irq_defs.h"
#include "irq_cpu.h"
#include "irq_pic.h"
#include "isr.h"

#include "../../stdint.h"
#include "../../stdio.h"
#include "../../x86/x86.h"
#include "../../devices/cpu/cpu.h"
#include "../../devices/i8259/i8259.h"

int_handle_wrapper gsi_handler_table[MAX_HANDLED_INT];
#define DEFAULT_REMAP_OFFSET 0x20

bool has_cpu, has_pic;
u8 handler_offset=0;
u8 int_remap_vector=DEFAULT_REMAP_OFFSET;
u8 int_vector_cnt;

static void irq_handler(registers_t* regs, void* ctx);
static u8 irq_get_offset();
static u8 irq_get_vec_cnt();

void IRQ_enable_interrupts()
{
    _x86_enable_interrupt();
}

void IRQ_disable_interrupts()
{
    _x86_disable_interrupt();
}

void IRQ_halt()
{
    _x86_halt();
}

void IRQ_end(int gsi)
{
    if (has_cpu) irq_cpu_end(gsi);
    else if (has_pic) irq_pic_end(gsi);
    else kprintf("Interrupt: EOI sending failed. API is inactive.\n");
}

void IRQ_init()
{
    has_cpu = irq_set_up_cpu();
    if (has_cpu)
    {
        kprintf("Interrupt: CPU driver will be used. Cleaning up...\n");
        has_pic = false;
        driver_unload("8259 PIC"); // Clean up unused driver
    }
    else 
    {
        has_pic = irq_set_up_pic(); 
        // Only use PIC as a "last resort", if CPU driver fails
        if (has_pic)
            kprintf("Interrupt: 8259 PIC driver will be used. Cleaning up...\n");
    }

    if (!has_cpu && !has_pic) 
    {
        kprintf("Interrupt: Init failed. No interrupts will be raised\n");
        return;
    }

    int_remap_vector = irq_get_offset();
    int_vector_cnt = irq_get_vec_cnt();
    handler_offset = int_remap_vector-DEFAULT_REMAP_OFFSET;

    for (int i=0;i<int_vector_cnt;i++)
    {
        ISR_reg_handler(int_remap_vector+i, irq_handler, NULL);
    }
    IRQ_enable_interrupts();
}

void IRQ_setup(int gsi_num, gsi_handler_t handler, void* ctx)
{
    if (has_cpu) irq_cpu_setup_int(gsi_num, handler, ctx);
    else if (has_pic) irq_pic_setup_int(gsi_num, handler, ctx);
    else kprintf("Interrupt: Routing failed. API is inactive.\n");
}

void IRQ_disable(int gsi_num)
{
    if (has_cpu) irq_cpu_disable_int(gsi_num);
    else if (has_pic) irq_pic_disable_int(gsi_num);
    else kprintf("Interrupt: Unrouting failed. API is inactive.\n");
}

void* IRQ_get_ctx(int gsi_num)
{
    if (gsi_num+handler_offset < 0 || gsi_num+handler_offset >= MAX_HANDLED_INT)
        return NULL;
    return gsi_handler_table[gsi_num+handler_offset].ctx;
}

/*-- STATIC FUNCTIONS --*/

static void irq_handler(registers_t* regs, void* ctx)
{
    (void)ctx; // Also, for a general handler like this, we won't need a context
    int gsi = regs->vector - int_remap_vector;

    if (gsi_handler_table[handler_offset+gsi].handler) 
    gsi_handler_table[handler_offset+gsi].handler(regs, gsi_handler_table[handler_offset+gsi].ctx);
    else
    {
        kprintf("Interrupt: External interrupt no. %d unhandled!", gsi);
    }

    IRQ_end(gsi);
}

static u8 irq_get_offset()
{
    if (has_cpu) irq_cpu_get_vector_offset();
    else if (has_pic) irq_pic_get_vector_offset();
    else kprintf("Interrupt: Get vector offset failed. API is inactive.\n");
}

static u8 irq_get_vec_cnt()
{
    if (has_cpu) irq_cpu_get_vector_cnt();
    else if (has_pic) irq_pic_get_vector_cnt();
    else kprintf("Interrupt: Get vector count failed. API is inactive.\n");
}
