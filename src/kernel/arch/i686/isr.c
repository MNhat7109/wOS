#include <kernel/arch/i686/isr.h>
#include <kernel/arch/i686/idt.h>
#include <kernel/debug.h>

#define MODULE_INT "INTERRUPT"
#define MODULE_EX "EXCEPTION"

typedef struct isr_wrapper_t
{
    isr_handler_cb_t callback;
    void* ctx;
} isr_wrapper_t;

static const char* const exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void isr_init_idt_entries();
void __attribute__((cdecl)) isr_generic_handler(register_state_t* reg_state, void* ctx);

isr_wrapper_t isr_handlers[256];

void isr_init()
{
    isr_init_idt_entries();

    for (u32 i=0;i<256;i++) idt_mark_present(i);
}

void __attribute__((cdecl)) isr_generic_handler(register_state_t* reg_state, void* ctx)
{
    (void)ctx;

    isr_wrapper_t* wrapper = &isr_handlers[reg_state->vector];
    if (wrapper->callback)
    {
        wrapper->callback(reg_state, wrapper->ctx);
        return;
    }

    if (reg_state->vector >= 32)
    {
        kdebugf(DEBUG_WARN, MODULE_INT, "Unhandled interrupt no. %u\n", reg_state->vector);
    }
    else
    {
        kdebugf(DEBUG_CRITICAL, MODULE_EX, "KERNEL PANIC!\n"
            "\t\tUnhandled exception no. %u: %s\n"
            "\t\tError code: %x\n\n"
            "\t\tRegister dump:\n"
            "\t\t  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n"
            "\t\t  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n", 
            reg_state->vector, exceptions[reg_state->vector],
            reg_state->error,
            reg_state->eax, reg_state->ebx, reg_state->ecx, reg_state->edx, reg_state->esi, reg_state->edi,
            reg_state->esp, reg_state->ebp, reg_state->eip, reg_state->eflags, reg_state->cs, reg_state->ds, reg_state->ss
        );

        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

void isr_register_handler(u8 vector, isr_handler_cb_t handler, void* handler_ctx)
{
    isr_wrapper_t* wrapper = &isr_handlers[vector];

    wrapper->callback = handler;
    wrapper->ctx = handler_ctx;
}