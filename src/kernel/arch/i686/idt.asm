bits 32

global idt_load_table
extern idtr
extern idt

; /**
; *
; * @brief Load the Interrupt Descriptor Table address.
; *
; * @clobbers eax
; */
idt_load_table:
    mov ax, 2047 ; Total entries of IDT: 256 * 8 bytes size = 2048 (bytes); 
    mov [idtr], ax

    mov eax, idt
    mov [idtr+2], eax

    lidt [idtr]
    ret
