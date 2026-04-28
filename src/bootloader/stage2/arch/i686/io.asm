bits 32

global i686_outb
i686_outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov al, [ebp+12]

    out dx, al

    mov esp, ebp
    pop ebp
    ret

global i686_inb
i686_inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in al, dx

    mov esp, ebp
    pop ebp
    ret

global i686_outl
i686_outl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov eax, [ebp+12]

    out dx, eax

    mov esp, ebp
    pop ebp
    ret

global i686_inl
i686_inl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in eax, dx

    mov esp, ebp
    pop ebp
    ret

global i686_insl
i686_insl:
    push ebp
    mov ebp, esp

    push edi
    ; Save ES register
    push es
    mov ax, [ebp+12]
    mov es, ax

    mov dx, [ebp+8]
    mov edi, [ebp+16]
    mov ecx, [ebp+20]
    rep insd

    pop es ; Restore ES
    pop edi
    mov esp, ebp
    pop ebp
    ret

global i686_outsl
i686_outsl:
    push ebp
    mov ebp, esp

    push esi
    ; Save DS register
    push ds
    mov ax, [ebp+12]
    mov ds, ax

    mov dx, [ebp+8]
    mov esi, [ebp+16]
    mov ecx, [ebp+20]
    rep outsd

    pop ds ; Restore DS
    pop esi
    mov esp, ebp
    pop ebp
    ret

global i686_insw
i686_insw:
    push ebp
    mov ebp, esp

    push edi
    ; Save ES register
    push es
    mov ax, [ebp+12]
    mov es, ax

    mov dx, [ebp+8]
    mov edi, [ebp+16]
    mov ecx, [ebp+20]
    rep insw

    pop es ; Restore ES
    pop edi
    mov esp, ebp
    pop ebp
    ret

global i686_outsw
i686_outsw:
    push ebp
    mov ebp, esp
    push esi
    ; Save DS register
    push ds
    mov ax, [ebp+12] ; DS is 0x10
    mov ds, ax ; Now ES is 0x10

    mov dx, [ebp+8]
    mov esi, [ebp+16]
    mov ecx, [ebp+20]
    rep outsw

    pop ds ; Restore DS
    pop esi
    mov esp, ebp
    pop ebp
    ret
