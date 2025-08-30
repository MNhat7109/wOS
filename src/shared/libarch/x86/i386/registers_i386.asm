bits 32

section .text

global _x86_halt
global _x86_enable_interrupt
global _x86_disable_interrupt

_x86_halt:
    hlt
    ret

_x86_enable_interrupt:
    sti
    ret

_x86_disable_interrupt:
    cli
    ret

