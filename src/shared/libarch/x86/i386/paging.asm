bits 32

section .text

global i386_load_paging
global i386_enable_paging
global i386_tlb_flush

i386_enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    jmp short .done
.done:
    ret

i386_load_paging:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

i386_tlb_flush:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    invlpg [eax]

    mov esp, ebp
    pop ebp
    ret