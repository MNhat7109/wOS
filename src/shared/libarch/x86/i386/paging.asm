bits 32

section .text

global i386_load_paging
global i386_enable_paging
global i386_enable_pae
global i386_tlb_flush
global i386_read_cr2

i386_enable_paging:
    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax

    push cs
    push .done
    retf

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

i386_read_cr2:
    push ebp
    mov ebp, esp

    push esi
    mov esi, [ebp+8]

    mov eax, cr2
    mov [esi], eax
    
    pop esi

    mov esp, ebp
    pop ebp
    ret

i386_enable_pae:
    mov eax, cr4
    ; Enable bit 5 in CR4 to enable PAE
    bts eax, 5
    mov cr4, eax
    ret