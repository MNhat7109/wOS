#include <kernel/irq.h>
#include <kernel/mmu_heap.h>

extern struct irq_data_t irq_data;

void irq_register(u8 irq, irq_dispatcher_cb_t cb, void* ctx)
{
    if (!irq_data.done_setup) return;

    irq_data.cur_ops->reg(irq, cb, ctx);
}

void irq_unregister(u8 irq, irq_dispatcher_cb_t cb)
{
    if (!irq_data.done_setup) return;
    irq_data.cur_ops->unreg(irq, cb);
}

void irq_ack(u8 irq)
{
    if (!irq_data.done_setup) return;
    irq_data.cur_ops->ack(irq);
}