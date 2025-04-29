; disk_check_ext: Check if BIOS supports the IBM Extension functions (EDD)
; Input: DX = Boot Drive
; Output: AL = 0 if successful or 1 if failed

disk_check_ext:
    push dx
    push bx
    mov bx, 0x55aa
    mov ah, 0x41
    stc
    int 0x13
    
    jnc .done
.unsupported:
    mov al, 1
.done:
    pop dx
    pop bx
    ret

; disk_read: Extended read sectors, if BIOS supports the Extension.
; Input: DX = Boot Drive
;        BX = Sector count
;        DI = Buffer address offset
;        CX = Buffer address segment
;        EAX = LBA (32 bit)
;        SP = Disk Address Packet
; Output: CF set = failure, CF clear = success

disk_read:
    push dx
    push si
    sub sp, 16
    mov bp, sp
    mov byte [bp], 16
    add bp, 1
    mov byte [bp], 0
    add bp, 1
    mov word [bp], bx
    add bp, 2
    mov word [bp], di
    add bp, 2
    mov word [bp], cx
    add bp, 2
    mov dword [bp], eax
    add bp, 4
    mov dword [bp], 0
    add bp, 4

.work:

    mov cx, 3

    mov si, sp
    mov ah, 0x42
    stc
    int 0x13

    jnc .done

.reset:
    xor ax, ax
    int 0x13
    dec cx
    test cx, cx
    jnz .work

.error:
    stc

.done:
    mov sp, bp
    pop si
    pop dx
    ret

