bits 32

global kswitchstack

kswitchstack:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov edx, [ebp+12]

    mov esp, ebp
    pop ebp

    mov esp, eax
    push edx
    jmp kresume

kresume:
    ret