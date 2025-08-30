bits 32

section .text

global _x86_cpuid
global _x86_rdmsr
global _x86_wrmsr
global _x86_has_cpuid

_x86_cpuid:
    push ebp
    mov ebp, esp

    push esi
    push ebx

    mov eax, [ebp+8] ; Save content of leaf (see arch/x86/common/msr.h) to EAX

    mov ecx, [ebp+12] ; Subleaf to ECX

    cpuid

; Save cpuid-loaded features to the respective params
    
    mov esi, [ebp+16]
    mov [esi], eax

    mov esi, [ebp+20]
    mov [esi], ebx
    
    mov esi, [ebp+24]
    mov [esi], ecx

    mov esi, [ebp+28]
    mov [esi], edx

    pop ebx
    pop esi

    mov esp, ebp
    pop ebp
    ret

_x86_rdmsr:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]
    rdmsr
    
    mov esp, ebp
    pop ebp
    ret

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

_x86_has_cpuid:
    pushfd ; Save EFLAGS

    pop eax ; And put it in EAX
    mov ecx, eax ; Copy to ECX

    xor eax, (1<<21) ; Try toggle the ID flag...
    push eax ; Save our modded EFLAGS
    
    popfd ; Load the modded EFLAGS

    ; Now, read that again. Does it change?
    pushfd

    pop eax
    xor eax, ecx ; Test with the original bit
    test eax, (1<<21) ; Does it change?

    jz .no_cpuid

    ; CPUID supported - toggle EAX to 1 to signal the higher-ups
    mov eax, 1
    jmp .done

.no_cpuid:
    ; No CPUID, clear EAX to 0
    xor eax, eax
    jmp .done

.done:
    ret