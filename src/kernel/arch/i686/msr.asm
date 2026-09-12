bits 32

global msr_read
global msr_write


; /**
; *
; * @brief Read value from Model Specific Register (MSR), if supported.
; *
; * @param msr
; *     Passed in [esp+4]
; *     
; * @return eax, edx
; * @clobber eax, edx, ecx
msr_read:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]
    rdmsr

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Write value to Model Specific Register (MSR), if supported.
; *
; * @param msr
; *     Passed in [esp+4]
; * @param value
; *     Passed in [esp+8..esp+12]
; *     
; * @clobber eax, edx, ecx
msr_write:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]
    mov eax, [ebp+12]
    mov edx, [ebp+16]
    wrmsr

    mov esp, ebp
    pop ebp
    ret