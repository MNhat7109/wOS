#include <kernel/interrupt/irq.h>
#include <kernel/arch/i686/isr.h>
#include <kernel/arch/i686/irq.h>
#include <kernel/arch/i686/state.h>
#include <kernel/interrupt/interrupt_vector.h>
#include <kernel/debug.h>

#define MAX_INTERRUPT_VECTORS 96
#define INT_VECTOR_OFFSET 0x20

static struct
{
    const irq_ops_t* cur_ops;
    bool done_setup;
} irq_data;

const irq_ops_t* apic_init() { return NULL;} // TODO: Implement APIC and its friends
const irq_ops_t* pic_init(u8 remap_offset);

void irq_load_ops(const irq_ops_t* ops)
{
    irq_data.cur_ops = ops;
    irq_data.done_setup = true;
}

irq_ops_t* irq_get_ops() { return (irq_data.done_setup)?irq_data.cur_ops:NULL;}

void irq_init()
{
    // 2 ways: APIC or fallback approach
    kdebugf(DEBUG_INFO, MODULE_IRQ, "Setting up i686 IRQ...\n");

    irq_ops_t* ops;
    // Check if APIC is initialized, fall back to PIC otherwise.
    ops = apic_init();
    if (ops) goto next;

    ops = pic_init(INT_VECTOR_OFFSET);
    if (!ops) return;

next:
    irq_load_ops(ops);
}