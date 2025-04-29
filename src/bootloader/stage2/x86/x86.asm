bits 32

global _x86_outb
_x86_outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov al, [ebp+12]

    out dx, al

    mov esp, ebp
    pop ebp
    ret

global _x86_inb
_x86_inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in al, dx

    mov esp, ebp
    pop ebp
    ret

global _x86_idt_load
_x86_idt_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    lidt [eax]
    
    mov esp, ebp
    pop ebp
    ret

global _x86_panic
_x86_panic:
    cli
    hlt

global _x86_enable_interrupt
_x86_enable_interrupt:
    sti
    ret

global _x86_disable_interrupt
_x86_disable_interrupt:
    cli
    ret