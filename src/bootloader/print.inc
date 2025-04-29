; print: Teletype string output function
; Input: SI = string
; Output: to screen

print:
    push si
    push ax
    push bx

.loop:
    lodsb ; Load 1 char from string to AL
    or al, al ; Is the character NULL?
    jz .done

    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp .loop

.done:
    pop bx
    pop ax
    pop si
    ret
