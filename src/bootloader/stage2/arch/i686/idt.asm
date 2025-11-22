bits 32

global i686_idt_load
i686_idt_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    lidt [eax]
    
    mov esp, ebp
    pop ebp
    ret
