bits 32

global _x86_GDT_load
_x86_GDT_load:
    push ebp
    mov ebp, esp
    push ebx
    cli
    
    mov eax, [ebp+8]
    lgdt [eax]

    mov eax, [ebp+12]
    mov bx, [ebp+16]
    push eax
    push .rel
    retf
.rel:
    mov ax, bx
    mov ds, ax
    mov es,ax
    mov fs, ax
    mov gs, ax
    mov ss,ax

    pop ebx
    mov esp, ebp
    pop ebp
    ret

global _x86_IDT_load
_x86_IDT_load:
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

global crash
crash:
    push ebp
    mov ebp, esp

    mov eax, 1337
    mov ebx, 0
    div ebx
    mov esp, ebp
    pop ebp
    ret

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

global _x86_enable_interrupt
_x86_enable_interrupt:
    sti
    ret