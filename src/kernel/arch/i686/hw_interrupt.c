#include <kernel/sys/hw_interrupt.h>
#include <kernel/sys/hw_int_defs.h>
#include <kernel/arch/i686/state.h>
#include <kernel/arch/i686/io.h>
#include <kernel/mmu_heap.h>
#include <stdint.h>
#include <string.h>

hw_int_data_t hw_int_data;

const hw_int_ops_t* pic_get_ops();

int hw_interrupt_init()
{
    // Init IOAPIC
    // If IOAPIC init() fails, fall back to i8259 PIC
    // If PIC fails too, ehhhh....
    hw_int_data.int_ops = pic_get_ops();

    int st = hw_int_data.int_ops->init(&hw_int_data);
    if (st < 0 || !hw_int_data.init) return -1;

    if (hw_int_data.handler_cnt == 0)
    {
        panic();
    }

    hw_int_data.hw_int_handler_table = (hw_int_wrapper_t*)mmu_heap_alloc(hw_int_data.handler_cnt*sizeof(hw_int_wrapper_t));
    for (usize i=0;i<hw_int_data.handler_cnt;i++) hw_int_data.hw_int_handler_table[i] = HANDLER_DEFAULT;
    __asm__ volatile("sti");
    return 0;
}

void hw_default_handler(register_state_t* regs, void* ctx)
{
    (void)ctx;
    if (!hw_int_data.init) return;

    int int_no = hw_int_data.int_ops->vector2intno(&hw_int_data, regs->vector);

    // Notify unhandled interrupts

    hw_interrupt_ack(int_no);
}

void* hw_interrupt_ctx(int int_no)
{
    if (!hw_int_data.init) return NULL;
    if (int_no >= hw_int_data.handler_cnt) return NULL;

    return hw_int_data.hw_int_handler_table[int_no].ctx;
}

void hw_interrupt_ack(int int_no)
{
    if (!hw_int_data.init) return;
    if (int_no >= hw_int_data.handler_cnt) return;

    hw_int_data.int_ops->ack(&hw_int_data, int_no);
}

void hw_interrupt_disable(int int_no)
{
    if (!hw_int_data.init) return;
    if (int_no >= hw_int_data.handler_cnt) return;

    hw_int_data.int_ops->disable(&hw_int_data, int_no);
}

void hw_interrupt_enable(int int_no)
{
    if (int_no >= hw_int_data.handler_cnt) return;
    if (!hw_int_data.int_ops) return;

    hw_int_data.int_ops->enable(&hw_int_data, int_no);
}

void hw_interrupt_enable_all()
{
    if (!hw_int_data.int_ops) return;
    hw_int_data.int_ops->enable_all(&hw_int_data);
}

void hw_interrupt_disable_all()
{
    if (!hw_int_data.int_ops) return;
    hw_int_data.int_ops->disable_all(&hw_int_data);
}

int hw_interrupt_register(int int_no, hw_int_handler_t handler, void* ctx)
{
    if (!hw_int_data.init) return -1;

    if (int_no >= hw_int_data.handler_cnt) return -1;
    if (!handler) return -1;

    hw_int_data.hw_int_handler_table[int_no] = (hw_int_wrapper_t){
        handler,
        ctx
    };
}

void hw_interrupt_unregister(int int_no)
{
    if (!hw_int_data.init) return -1;

    if (int_no >= hw_int_data.handler_cnt) return -1;

    hw_int_data.hw_int_handler_table[int_no] = HANDLER_DEFAULT;
}