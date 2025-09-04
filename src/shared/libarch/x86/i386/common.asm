bits 32

global i386_GDT_load
global i386_IDT_load
global i386_TSS_flush
global i386_TSS_save_sp

i386_GDT_load:
    push ebp
    mov ebp, esp
    push ebx
    cli
    
    mov eax, [ebp+8]
    lgdt [eax]

    xor eax,eax
    mov ax, [ebp+12]
    mov bx, [ebp+16]
    push ax
    push dword .rel
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

i386_IDT_load:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp+8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret

i386_TSS_flush:
    mov eax, (5*8) | 0
    ltr ax
    ret

i386_TSS_save_sp:
    mov eax, esp
    push esi
    mov esi, [esp+8]
    mov [esi], eax
    pop esi
    ret
