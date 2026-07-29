bits 32

global outb
global inb
global outl
global inl
global outw
global inw
global panic

; /**
; *
; * @brief Write 8-bit value to port
; *
; * @param port
; *    Port number to write to.
; *    Passed in: [esp+4]
; * @param value
; *    8-bit value to write to the port.
; *    Passed in: [esp+8]
; * @clobbers eax, edx
; */
outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov al, [ebp+12]

    out dx, al

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Read 8-bit value from port
; *
; * @param port
; *    Port number to read from.
; *    Passed in: [esp+4]
; * @return eax
; */
inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in al, dx

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Write 32-bit value to port
; *
; * @param port
; *    Port number to write to.
; *    Passed in: [esp+4]
; * @param value
; *    32-bit value to write to the port.
; *    Passed in: [esp+8]
; * @clobbers eax, edx
; */
outl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov eax, [ebp+12]

    out dx, eax

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Read 32-bit value from port
; *
; * @param port
; *    Port number to read from.
; *    Passed in: [esp+4]
; * @return eax
; */
inl:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in eax, dx

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Write 16-bit value to port
; *
; * @param port
; *    Port number to write to.
; *    Passed in: [esp+4]
; * @param value
; *    16-bit value to write to the port.
; *    Passed in: [esp+8]
; * @clobbers eax, edx
; */
outw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]
    mov ax, [ebp+12]

    out dx, ax

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Read 16-bit value from port
; *
; * @param port
; *    Port number to read from.
; *    Passed in: [esp+4]
; * @return eax
; */
inw:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]

    xor eax,eax
    in ax, dx

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Stop CPU execution indefinitely
; *
; */
panic:
    cli
    hlt
