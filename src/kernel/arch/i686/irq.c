#include <kernel/irq.h>
#include <kernel/arch/i686/isr.h>
#include <kernel/arch/i686/irq.h>
#include <kernel/arch/i686/state.h>
#include <kernel/debug.h>

#define MAX_INTERRUPT_VECTORS 96
#define INT_VECTOR_OFFSET 0x20

struct irq_data_t irq_data;

int apic_init() { return -1;} // TODO: Implement APIC and its friends
int pic_init(u8 remap_offset);

int irq_vector_follow_chain(u8 vector, register_state_t* regs);
void irq_default_dispatcher(register_state_t* regs, void* ctx);

int irq_vector_alloc_init(int vector_offset, int vector_count);

void irq_load_ops(const irq_ops_t* ops)
{
    irq_data.cur_ops = ops;
    irq_data.done_setup = true;
}

void irq_init()
{
    // 2 ways: APIC or fallback approach
    kdebugf(DEBUG_INFO, MODULE_IRQ, "Setting up i686 IRQ...\n");

    // Check if APIC is initialized, fall back to PIC otherwise.
    int st = apic_init();
    if (st == 0) goto isr_fillup;

    st = pic_init(INT_VECTOR_OFFSET);
    if (st < 0) return;

    // 96 interrupt vectors (0x20 to 0x7F) will be used for IRQ handling
    // Fill the 96 corresponding ISR entries with the default IRQ dispatch.

isr_fillup:
    for (u8 v=0x20;v<=0x7F;v++)
    {
        isr_register_handler(v, irq_default_dispatcher, NULL);
    }

    // Init the interrupt vector allocator.
    irq_vector_alloc_init(INT_VECTOR_OFFSET, MAX_INTERRUPT_VECTORS);

    // Enable interrupts
    __asm__ volatile("sti");
}

// NOTE: Because we're following an interrupt dispatch chain to acknowledge one interrupt, maybe it is better to 
// ack interrupts while on the chain

void irq_default_dispatcher(register_state_t* regs, void* ctx)
{
    // Traverse for irq
    int irq_count = irq_vector_follow_chain(regs->vector, regs);

    if (irq_count==0)
    {
        // debug: Unhandled interrupt(s) at vector....
    }
}