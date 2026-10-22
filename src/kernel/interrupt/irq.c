#include <kernel/interrupt/irq.h>
#include <kernel/mmu_heap.h>

irq_ops_t* irq_get_ops();

void irq_register(u8 irq, int_dispatcher_cb_t cb, void* ctx)
{
    irq_ops_t* ops = irq_get_ops();
    if (!ops) return;

    ops->reg(irq, cb, ctx);
}

void irq_unregister(u8 irq, int_dispatcher_cb_t cb)
{
    irq_ops_t* ops = irq_get_ops();
    if (!ops) return;

    ops->unreg(irq, cb);
}