bits 32

global cpuid_check
global cpuidex

extern cpuid_notify_error
extern cpuid_notify_unsupported

; TODO: This function will only be temporary, will be changed after IDT is implemented
; /**
; *
; * @brief Check if CPUID can be used
; *
; * @clobbers eax
; * @return eax
; */
cpuid_check:
    pushfd ; Save EFLAGS
    pushfd ; Store EFLAGS
    xor dword [esp], (1<<21) ; Change ID bit
    popfd ; Load the new flags
    pushfd ; Then store the current EFLAGS after change
    pop eax ; And move it to EAX
    xor eax, [esp] ; Compare EAX with the previously saved bit
    popfd ; Restore the original flags
    shr eax, 21
    and eax, 1 ; EAX=0 -> No CPUID, EAX=1 -> CPUID
    test eax, eax
    jnz .done
    call cpuid_notify_unsupported ; Holler out loud to let the users know
.done:
    mov [cpuid_supported], eax
    ret

; /**
; *
; * @brief Run CPU Identification, with extended subfunctions
; *
; * @param function
; *    Function (feature leaf) number
; *    Passed in: [esp+4]
; * @param subfunction
; *    Subfunction (subleaf) number
; *    Passed in: [esp+8]
; * @param info_out[4]
; *    CPU feature results
; *    Passed in: [esp+12..esp+24]
; * @clobbers eax, ebx, ecx, edx
; */
cpuidex:
    push ebp
    mov ebp, esp

    push ebx
    push edi

    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov eax, cpuid_supported
    test eax, eax
    jz .error

    mov eax, [ebp+8]
    mov ecx, [ebp+12]
    cpuid

    mov edi, [ebp+16]
    mov [edi  ], eax
    mov [edi+4], ebx
    mov [edi+8], ecx
    mov [edi+12], edx
    jmp .done
    
.error:
    call cpuid_notify_error ; Holler out loud to let the users know

.done:
    pop edi
    pop ebx

    mov esp, ebp
    pop ebp
    ret

cpuid_supported: db 0