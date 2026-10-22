#include <kernel/interrupt/interrupt_vector.h>
#include <kernel/arch/i686/state.h>
#include <kernel/arch/i686/isr.h>

#define MAX_INTERRUPT_VECTORS 96
#define INT_VECTOR_OFFSET 0x20

void interrupt_default_dispatcher(register_state_t* regs, void* ctx);
int interrupt_vector_alloc_init(int vector_offset, int vector_count);
int interrupt_vector_follow_dispatch_chain(int vector, register_state_t* regs);

void interrupt_vector_init()
{
    // Init interrupt vector alloc
    int st = interrupt_vector_alloc_init(INT_VECTOR_OFFSET, MAX_INTERRUPT_VECTORS);
    if (st < 0) return;
    
    // Fill up dispatchers
    for (u8 v=0x20;v<=0x7F;v++)
    {
        isr_register_handler(v, interrupt_default_dispatcher, NULL);
    }

    // Enable hardware interrupt
    __asm__ volatile("sti");
}

void interrupt_default_dispatcher(register_state_t* regs, void* ctx)
{
    // Traverse for irq
    int irq_count = interrupt_vector_follow_dispatch_chain(regs->vector, regs);

    if (irq_count==0)
    {
        // debug: Unhandled interrupt(s) at vector....
    }
}
