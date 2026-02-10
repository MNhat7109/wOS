bits 32

global gdt_load_table
global gdt_reload_segs
extern gdtr
extern gdt

; /**
; *
; * @brief Load the Global Descriptor Table address.
; *
; * @clobbers eax
; */
gdt_load_table:
    mov ax, 47 ; Total entries of GDT: 6 (NULL included) * 8 bytes size = 48 (bytes); 
    mov [gdtr], ax

    mov eax, gdt
    mov [gdtr+2], eax

    lgdt [gdtr]
    ret

; /**
; *
; * @brief Reload code and data segments.
; *
; * @param cs
; *    Code segment offset in the GDT to switch to.
; *    Passed in: [esp+4]
; * @param ds
; *    Data segment offset in the GDT to switch to.
; *    Passed in: [esp+8]
; * @clobbers eax, edx
; */
gdt_reload_segs:
    push ebp
    mov ebp, esp

    mov ax, [ebp+8]
    mov dx, [ebp+12]
    push ax
    push dword .rel
    retf
.rel:
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx

    mov esp, ebp
    pop ebp
    ret