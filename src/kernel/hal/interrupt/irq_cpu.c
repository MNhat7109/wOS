#include "irq_cpu.h"
#include "irq_utils.h"
#include "irq_defs.h"

#include "../../devices/cpu/cpu.h"
#include "../../stdio.h"

#define MAX_HANDLED_GSI 0x50
#define APIC_REMAP_OFFSET 0x30

struct cpu_driver_t* cpu_dev;
extern u8 handler_offset;

bool irq_set_up_cpu()
{
    cpu_dev = (struct cpu_driver_t*)driver_get("CPU");

    if (!cpu_dev)
    {
        kprintf("Interrupt: CPU driver not found\n");
        return false;
    }
    if (!driver_run((struct generic_driver_t*)cpu_dev))
    {
        kprintf("Interrupt: CPU driver failed\n");
        driver_unload("CPU");
        return false;
    }

    return true;
}

void irq_cpu_setup_int(int interrupt, gsi_handler_t handler, void* ctx)
{
    if (interrupt < 0 || interrupt > MAX_HANDLED_GSI) return;

    u8 fixed_int = cpu_dev->normalize_irq(interrupt); // This will convert any legacy IRQs to their mapped GSIs
    u8 core_id = cpu_dev->get_core_id();
    
    irq_load_handler(fixed_int+handler_offset, handler, ctx);
    cpu_dev->redirect_gsi(fixed_int, APIC_REMAP_OFFSET+fixed_int, core_id);
}

void irq_cpu_disable_int(int interrupt)
{
    if (interrupt < 0 || interrupt > MAX_HANDLED_GSI) return;

    u8 fixed_int = cpu_dev->normalize_irq(interrupt); // This will convert any legacy IRQs to their mapped GSIs
    
    irq_unload_handler(fixed_int+handler_offset);
    cpu_dev->cut_gsi(fixed_int);
}

void irq_cpu_end(int interrupt)
{
    // The reason for the parameter is to maintain the
    // compatibility of ARM64.

    // I could've just put any number in it
    // And in x86, the driver won't care.
    
    cpu_dev->send_eoi(interrupt);
}

u8 irq_cpu_get_vector_cnt()
{
    return MAX_HANDLED_GSI;
}

u8 irq_cpu_get_vector_offset()
{
    return APIC_REMAP_OFFSET;
}