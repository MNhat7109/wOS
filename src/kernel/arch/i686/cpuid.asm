bits 32

global cpuid_check
global cpuid_ex

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
cpuid_ex:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov ecx, [ebp+12]
    cpuid

    mov [ebp+16], eax
    mov [ebp+20], ebx
    mov [ebp+24], ecx
    mov [ebp+28], edx

    mov esp, ebp
    pop ebp
    ret