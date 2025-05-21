#include "isr.h"
#include "idt.h"
#include "../../stdio.h"
#include "../../x86/x86.h"

isr_handler_t isr_handler_table[256];

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
    "Control Protection Exception ",
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

void ISR_init_gates();

void ISR_init()
{
    ISR_init_gates();
    for (int i=0;i<256;i++)
    {
        IDT_set_entry(i);
    }
}

void __attribute__((cdecl)) ISR_handler(registers_t* regs)
{
    // There's a handler for this interrupt, use it.
    kprintf("H\n");
    if (isr_handler_table[regs->vector])
        isr_handler_table[regs->vector](regs);
    else if (regs->vector >= 32)
        kprintf("Interrupt %u unhandled!\n", regs->vector);
    else
    {
        kprintf("KERNEL PANIC!\n");
        kprintf("Unhandled exception no. %u: %s\n", regs->vector, exceptions[regs->vector]);
        kprintf("Error code: %x\n\n", 
            regs->error);
        kprintf("Register dump:\n");
        kprintf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n", 
            regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        kprintf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
            regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        _x86_panic();
    }
}

void ISR_reg_handler(u8 vector, isr_handler_t handler)
{
    isr_handler_table[vector] = handler;
    IDT_set_entry(vector);
}