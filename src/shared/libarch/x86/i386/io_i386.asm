bits 32

section .text

global _x86_inb
global _x86_outb

global _x86_inw
global _x86_outw

global _x86_inw
global _x86_outw

_x86_inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in al, dx

    mov esp, ebp
    pop ebp
    ret

_x86_outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov al, [ebp+12]

    out dx, al

    mov esp, ebp
    pop ebp
    ret

_x86_inw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in ax, dx

    mov esp, ebp
    pop ebp
    ret

_x86_outw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov ax, [ebp+12]

    out dx, ax

    mov esp, ebp
    pop ebp
    ret

_x86_inl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in eax, dx

    mov esp, ebp
    pop ebp
    ret

_x86_outl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov eax, [ebp+12]

    out dx, eax

    mov esp, ebp
    pop ebp
    ret
