#include <kernel/sys/hw_interrupt.h>
#include <kernel/sys/hw_int_defs.h>
#include <kernel/arch/i686/state.h>
#include <kernel/arch/i686/io.h>
#include <kernel/mmu_heap.h>
#include <stdint.h>
#include <string.h>

typedef struct hw_int_data_t
{
    usize handler_cnt;
    hw_int_wrapper_t* hw_int_handler_table;
    bool init;
    const hw_int_ops_t* int_ops;
} hw_int_data_t;

hw_int_data_t hw_int_data;


int hw_interrupt_init()
{
    // Fill 96 vectors with the default handler

    // Initialize array of 96 handler linked lists

    // 
}

int int_vector_alloc()
{

}

void int_vector_free(int vector, int int_no)
{
    
}

int int_vector_find(int vector)
{

}

void hw_default_handler(register_state_t* regs, void* ctx)
{
    (void)ctx;
    if (!hw_int_data.init) return;

    int int_no = int_vector_find(regs->vector);

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

    hw_int_data.int_ops->ack(int_no);
}

void hw_interrupt_disable(int int_no)
{
    if (!hw_int_data.init) return;
    if (int_no >= hw_int_data.handler_cnt) return;

    hw_int_data.int_ops->disable(int_no);
}

void hw_interrupt_enable(int int_no)
{
    if (int_no >= hw_int_data.handler_cnt) return;
    if (!hw_int_data.int_ops) return;

    hw_int_data.int_ops->enable(int_no);
}

void hw_interrupt_enable_all()
{
    if (!hw_int_data.int_ops) return;
    hw_int_data.int_ops->enable_all();
}

void hw_interrupt_disable_all()
{
    if (!hw_int_data.int_ops) return;
    hw_int_data.int_ops->disable_all();
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