#include "isr.h"
#include "idt.h"
#include "irq.h"
#include "../../stdio.h"

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

void i686_isr_init_gates();

void i686_isr_init()
{
    i686_isr_init_gates();
    for (int i=0;i<256;i++)
    {
        i686_idt_set_entry(i);
    }
    i686_idt_clear_entry(0x80);
}

void __attribute__((cdecl)) i686_isr_handler(registers_t* regs)
{
    // There's a handler for this interrupt, use it.
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
        i686_panic();
    }
}

void i686_isr_register_handler(u8 vector, isr_handler_t handler)
{
    isr_handler_table[vector] = handler;
    i686_idt_set_entry(vector);
}