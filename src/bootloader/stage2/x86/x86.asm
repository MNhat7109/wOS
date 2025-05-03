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

global _x86_outl
_x86_outl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov eax, [ebp+12]

    out dx, eax

    mov esp, ebp
    pop ebp
    ret

global _x86_inl
_x86_inl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in eax, dx

    mov esp, ebp
    pop ebp
    ret

global _x86_insl
_x86_insl:
    push ebp
    mov ebp, esp
    ; Save ES register
    push es
    mov ax, [ebp+12]
    mov es, ax

    mov dx, [ebp+8]
    mov edi, [ebp+16]
    mov ecx, [ebp+20]
    rep insd

    pop es ; Restore ES
    mov esp, ebp
    pop ebp
    ret

global _x86_outsl
_x86_outsl:
    push ebp
    mov ebp, esp
    ; Save DS register
    push ds
    mov ax, [ebp+12]
    mov ds, ax

    mov dx, [ebp+8]
    mov esi, [ebp+16]
    mov ecx, [ebp+20]
    rep outsd

    pop ds ; Restore DS
    mov esp, ebp
    pop ebp
    ret

global _x86_insw
_x86_insw:
    push ebp
    mov ebp, esp
    ; Save ES register
    push es
    mov ax, [ebp+12]
    mov es, ax

    mov dx, [ebp+8]
    mov edi, [ebp+16]
    mov ecx, [ebp+20]
    rep insw

    pop es ; Restore ES
    mov esp, ebp
    pop ebp
    ret

global _x86_outsw
_x86_outsw:
    push ebp
    mov ebp, esp
    ; Save DS register
    push ds
    mov ax, [ebp+12] ; DS is 0x10
    mov ds, ax ; Now ES is 0x10

    mov dx, [ebp+8]
    mov esi, [ebp+16]
    mov ecx, [ebp+20]
    rep outsw

    pop ds ; Restore DS
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

global _x86_halt
_x86_halt:
    hlt