bits 32

%macro _x86_enter_real_mode 0
    [bits 32]
    jmp word 0x18:.pmode16

.pmode16:
    [bits 16]
    mov eax, cr0
    and al, ~1
    mov cr0, eax
    sidt [old_idt]
    lidt [ivt]
    jmp word 0:.rmode

.rmode:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax

    sti
%endmacro
ivt: dw 0x3ff
     dd 0

%macro _x86_enter_protected_mode 0
    cli

    mov eax, cr0
    or al, 1
    mov cr0, eax
    lidt [old_idt]

    jmp dword 0x08:.pmode

.pmode:
    [bits 32]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
%endmacro
old_idt: dw 0
         dd 0

        
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
    ret

global _x86_VBE_get_info
_x86_VBE_get_info:
    [bits 32]
    push ebp
    mov ebp, esp

    _x86_enter_real_mode
    push es
    push edi

    mov di, [bp+8]
    mov es, di
    xor edi, edi
    mov di, [bp+10]
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x4F
    jne .fail
    mov eax, 1
    jmp .after
.fail:
    xor eax, eax
.after:
    pop edi
    pop es
    push eax
    _x86_enter_protected_mode
    pop eax
    mov esp, ebp
    pop ebp
    ret

global _x86_VBE_get_mode
_x86_VBE_get_mode:
    [bits 32]
    push ebp
    mov ebp, esp

    _x86_enter_real_mode
    push es
    push edi
    push ecx

    mov di, [bp+8]
    mov es, di
    xor edi, edi
    mov di, [bp+10]
    mov ecx, [bp+12]
    mov ax, 0x4F01
    int 0x10
    cmp ax, 0x4F
    jne .fail
    mov eax, 1
    jmp .after
.fail:
    xor eax, eax
.after:
    pop ecx
    pop edi
    pop es
    push eax
    _x86_enter_protected_mode
    pop eax
    mov esp, ebp
    pop ebp
    ret

global _x86_VBE_set_mode
_x86_VBE_set_mode:
    [bits 32]
    push ebp
    mov ebp, esp

    _x86_enter_real_mode
    push bx

    mov bx, [bp+8]
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x4F
    jne .fail
    mov eax, 1
    jmp .after
.fail:
    xor eax, eax
.after:
    pop bx
    push eax
    _x86_enter_protected_mode
    pop eax
    mov esp, ebp
    pop ebp
    ret
