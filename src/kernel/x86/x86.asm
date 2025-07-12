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

global _x86_TSS_flush
_x86_TSS_flush:
    mov eax, (5*8) | 0
    ltr ax
    ret

global _x86_TSS_save_esp0
_x86_TSS_save_esp0:
    mov eax, esp
    push esi
    mov esi, [esp+8]
    mov [esi], eax
    pop esi
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

global _x86_outw
_x86_outw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov ax, [ebp+12]

    out dx, ax

    mov esp, ebp
    pop ebp
    ret

global _x86_inw
_x86_inw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in ax, dx

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

global _x86_load_paging
_x86_load_paging:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

global _x86_enable_paging
_x86_enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    jmp short .next

.next:
    ret

global _x86_tlb_flush
_x86_tlb_flush:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    invlpg [eax]
    mov esp, ebp
    pop ebp
    ret

global _x86_multitasking_save_regs
_x86_multitasking_save_regs:
    mov eax, esp
    push esi
    mov esi, [esp+8]
    mov [esi], eax


    mov esi, [esp+12]
    mov eax, cr3
    mov [esi], eax

    pop esi
    ret

extern current_process
extern tss_entry
global _x86_multitasking_switch_task
_x86_multitasking_switch_task:
    ; cli

    push ebx
    push esi
    push edi
    push ebp
    

    mov edi, [current_process]
    mov [edi+4], esp


    mov esi, [esp+20] ; Get the address of first param
    mov [current_process], esi
    
    ; push eax
    mov eax, [esi+12]
    mov ebx, [esi+8]
    mov [tss_entry+4], ebx
    mov edx, [esi+4]
    mov ecx, cr3
    cmp eax, ecx
    ; cli
    ; hlt
    je .done
    mov cr3, eax
    cli
    hlt

.done:
    pop ebp
    pop edi
    pop esi
    pop ebx

    mov esp, edx

    ; sti
    ret

global _x86_get_cpuid
_x86_get_cpuid:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    cpuid

    mov esp, ebp
    pop ebp
    ret

global _x86_rdmsr
_x86_rdmsr:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]
    rdmsr
    
    mov esp, ebp
    pop ebp
    ret

global _x86_wrmsr
_x86_wrmsr:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]
    mov eax, [ebp+12]
    mov edx, [ebp+16]
    wrmsr

    mov esp, ebp
    pop ebp
    ret