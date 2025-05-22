[bits 16]

section .entry

extern __bss_start
extern __end

extern start
global entry16

entry16:
    cli
    mov [bootDrive], dl
    mov [partOffset], bx


    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0

    call do_e820

    xor eax, eax
    call test_a20_gate
    test ax, ax
    jnz .GotoPM
    call a20_enable


.GotoPM:
    cli
    ; Load GDT
    xor ax, ax
    mov ds, ax
    call load_gdt
    mov eax, cr0
    or al, 1
    mov cr0, eax


    jmp dword 0x08:.main_32

.main_32:
    [bits 32]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax


    mov edi, 0xB8000
    mov ax, 0
    mov cx, 2000
    rep stosw
    
    xor edx, edx
    xor ebx, ebx
    mov dl, [bootDrive]
    mov bx, [partOffset]
    ; Fuck it, just forgot to push the parameters to the function start
    ; Absolute fucking crapass
    push 0x50000
    push ebx
    push edx
    call start
    
    cli
    hlt


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Helper functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

test_a20_gate:
    [bits 16]
    ; Basically check if value at address 0000:0500 != ffff:0510
    cli
    pushf
    push es
    push ds
    push si
    push di

.work:
    xor ax, ax
    mov ds, ax
    mov si, 0x500

    not ax
    mov es, ax
    mov di, 0x510

    mov al, [ds:si]
    mov byte [.BufferWithin], al
    mov al, [es:di]
    mov byte [.BufferOutside], al

    mov ah, 1
    mov byte [ds:si], 0
    mov byte [es:di], 1
    mov al, [es:di]
    cmp al, [ds:si]
    jne .done
    dec ah
.done:
    mov al, [.BufferOutside]
    mov [es:di], al
    mov al, [.BufferWithin]
    mov [ds:si], al
    shr ax, 8
    pop di
    pop si
    pop ds
    pop es
    popf
    ret

.BufferWithin: db 0
.BufferOutside: db 0

a20_enable:
    [bits 16]
    cli
    call a20_wait1
    mov al, 0xAD
    out 0x64, al

    call a20_wait1
    mov al, 0xD0
    out 0x64, al

    call a20_wait2
    in al, 0x60
    push eax

    call a20_wait1
    pop eax
    or al, 2
    out 0x60, al

    call a20_wait1
    mov al, 0xAE
    out 0x64, al

    call a20_wait1
    ret

a20_wait1:
    in al, 0x64
    test al, 1
    jnz a20_wait1
    ret

a20_wait2:
    in al, 0x64
    test al, 2
    jnz a20_wait2
    ret

entry_cnt equ MEMORY_MAP_OFFSET
do_e820:
    [bits 16]
    push es
    push ebx
    push ecx
    push edx
    mov di, MEMORY_MAP_SEGMENT
    mov es, di
    mov di, MEMORY_MAP_OFFSET
    add di, 4
    xor bp, bp

    xor ebx, ebx
    mov edx, 0x0534D4150
    mov eax, 0xE820
    mov dword [es:di+20], 1 ; Force ACPI 3.0 Extended attributes
    mov ecx, 24
    int 0x15
    jc .failed

    mov edx, 0x0534D4150
    cmp eax, edx
    jne .failed

    test ebx, ebx
    je .failed
    jmp .jin

.loop:
    mov eax, 0xE820
    mov dword [es:di+20], 1 ; Force ACPI 3.0 Extended attributes
    mov ecx, 24
    int 0x15
    jc .finish
    mov edx, 0x0534D4150

.jin:
    jcxz .skip
    cmp cl, 20
    jbe .noext
    test byte [es:di+20], 1
    je .skip

.noext:
    mov ecx, [es:di+8]
    or ecx, [es:di+12]
    jz .skip
    inc bp
    add di, 24

.skip:
    test ebx,ebx
    jne .loop

.finish:
    mov [es:entry_cnt], bp
    pop edx
    pop ecx
    pop ebx
    pop es
    clc
    ret

.failed:
    pop edx
    pop ecx
    pop ebx
    pop es
    stc
    ret

load_gdt:
    [bits 16]
    lgdt [gdtr]
    ret

gdt:
    dq 0
    
    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 00001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 00001111b
    db 0

gdtr: dw gdtr-gdt-1
     dd gdt



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bootDrive: db 0
partOffset: db 0
MEMORY_MAP_SEGMENT equ 0x5000
MEMORY_MAP_OFFSET equ 0