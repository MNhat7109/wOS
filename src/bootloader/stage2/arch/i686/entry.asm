[bits 16]

section .entry

extern __bss_start
extern __end

extern start
global entry16

entry16:
    cli
    mov [bootDrive], dl
    mov [partOffset], bx


    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0

    call do_e820

; Copy 'VBE2' to VBEInfo
    push dword 'VBE2'
    mov si, sp
    mov di, VBEInfo
    mov cx, 4
    repe movsb
    add sp, 4

    push VBEInfo
    push ds
    call vesa_vbe_get_info

; Compare word to 'VESA'
    push dword 'VESA'
    mov si, sp
    mov di, VBEInfo
    mov cx, 4
    repe cmpsb

    test cx, cx
    je .has_VESA
; No VESA for us =((
; Erase the whole screen, and jump to A20 enabling
; Our default resolution pre-VESA is 80x25 char, so we will write 80x25 space chars to clear
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov cx, 80*25       ; number of character cells
    mov ax, 0x0720      ; attribute=07h, character=' ' (space)
    rep stosw

    ; Also, reset cursor pos
    mov dx, 0 ; row = 0, col = 0
    mov bh, 0 ; page 0
    mov ah, 0x02 ; Set cursor to 0
    int 0x10
    jmp .a20_enable
.has_VESA:
; Before setting anything, we must copy the VGA Font stored in the BIOS first.
; Since we are still in real mode, one BIOS call is enough. Easy and simple.
; We will use an 8x16 font, 256 glyphs. That makes the total buffer 4096 bytes in capacity.
    push di
    push si
    push ds ; This for our temp buffer
    push es ; This for the output of BIOS call

    mov ax, 0x1130 ; Get Font Info
    mov bh, 0x06 ; ROM 8x16 font (MCGA, VGA)
    int 0x10

    ; Now the font location will be at ES:BP, we want to copy 4096 bytes from that to our desired buffer.
    push es
    pop ds ; DS is now ES
    pop es ; and vice versa
    mov si, bp ; Source at ES:BP
    mov di, tempGlyphBuffer ; Dest
    mov cx, 256*16/4 ; 1024 DWORDs => 4096 bytes
    rep movsd
    pop ds ; Restore old DS
    pop si
    pop di

; Set screen res to 1280x800, color depth 32 bit
    push 32
    push 800
    push 1280
    call vesa_vbe_scan_mode ; Check if there's one
    or ax, 0x4000 ; Enable LFB

    push ax
    call vesa_vbe_set_mode

    ; Store info on the videoBlock
    mov eax, [VBEModeBlock+0x28] ; Framebuffer base
    mov dword [videoBlock], eax
    xor eax, eax
    
    mov ax, [VBEModeBlock+0x14] ; Height of res
    mov dword [videoBlock+0x8], eax
    xor eax, eax
    
    mov ax, [VBEModeBlock+0x12] ; Width of res
    mov dword [videoBlock+0xC], eax
    xor eax, eax
    
    mov dword [videoBlock+0x10], 32 ; BPP of res
    
    mov ax, [VBEModeBlock+0x10]
    mov dword [videoBlock+0x14], eax ; Pitch of res

    mov eax, [videoBlock+0x10]
    mov edx, [videoBlock+0x14]
    mul dx
    shl edx, 16
    add eax, edx ; Size of framebuffer = Height * Pitch
    mov eax, [videoBlock+0x18]

; One last job: copy the whole MBR sector for convienience later,
; when we need to determine where our boot disk resides in.
    mov dl, [bootDrive]
    
    sub sp, 16
    mov bp, sp
    mov byte [bp], 16 ; Size of DAP
    mov byte [bp+1], 0 ; Reserved field
    mov word [bp+2], 1 ; Read 1 sector
    mov word [bp+4], mbrBuffer ; Offset
    mov cx, ds
    mov word [bp+6], cx ; Segment
    mov dword [bp+8], 0 ; LBA low
    mov dword [bp+12], 0 ; LBA hi

    mov cx, 3
.read:
    push ds
    mov ax, ss
    mov ds, ax
    mov si, bp
    mov ah, 0x42
    int 0x13
    pop ds
    jnc .done

.reset:
    xor ax, ax
    int 0x13
    dec cx
    test cx, cx
    jnz .read
.error:
    cli
    hlt
.done:
    add sp, 16

; Finally, go 32 bit. BIOS interrupts go bye bye.
.a20_enable:
    xor eax, eax
    call test_a20_gate
    test ax, ax
    jnz .GotoPM
    call a20_enable
.GotoPM:
    cli
    ; Load GDT
    xor ax, ax
    mov ds, ax
    call load_gdt
    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp dword 0x08:.main_32

.main_32:
    [bits 32]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Now that we're in 32-bit mode, we can save font info
    ; We're doing this, so that the address at our temporary glyph
    ; is a flat one, not segment:offset
    
    mov byte [fontBlock.width], 8
    mov byte [fontBlock.height], 16
    mov word [fontBlock.glyph_count], 256
    mov dword [fontBlock.glyph_buffer], tempGlyphBuffer
    
    xor edx, edx
    xor ebx, ebx
    mov dl, [bootDrive]
    mov ebx, mbrBuffer
    mov esi, memInfoBlock
    push esi
    mov esi, fontBlock
    push esi
    mov esi, videoBlock
    push esi
    push ebx
    push edx
    call start
    
    cli
    hlt


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Helper functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

test_a20_gate:
    [bits 16]
    ; Basically check if value at address 0000:0500 != ffff:0510
    cli
    pushf
    push es
    push ds
    push si
    push di

.work:
    xor ax, ax
    mov ds, ax
    mov si, 0x500

    not ax
    mov es, ax
    mov di, 0x510

    mov al, [ds:si]
    mov byte [.BufferWithin], al
    mov al, [es:di]
    mov byte [.BufferOutside], al

    mov ah, 1
    mov byte [ds:si], 0
    mov byte [es:di], 1
    mov al, [es:di]
    cmp al, [ds:si]
    jne .done
    dec ah
.done:
    mov al, [.BufferOutside]
    mov [es:di], al
    mov al, [.BufferWithin]
    mov [ds:si], al
    shr ax, 8
    pop di
    pop si
    pop ds
    pop es
    popf
    ret

.BufferWithin: db 0
.BufferOutside: db 0

a20_enable:
    [bits 16]
    cli
    call a20_wait1
    mov al, 0xAD
    out 0x64, al

    call a20_wait1
    mov al, 0xD0
    out 0x64, al

    call a20_wait2
    in al, 0x60
    push eax

    call a20_wait1
    pop eax
    or al, 2
    out 0x60, al

    call a20_wait1
    mov al, 0xAE
    out 0x64, al

    call a20_wait1
    ret

a20_wait1:
    in al, 0x64
    test al, 1
    jnz a20_wait1
    ret

a20_wait2:
    in al, 0x64
    test al, 2
    jnz a20_wait2
    ret

do_e820:
    [bits 16]
    push es
    push ebx
    push ecx
    push edx
    mov ax, ds
    mov es, ax
    mov di, memInfoBlock
    add di, 4
    xor bp, bp

    xor ebx, ebx
    mov edx, 0x0534D4150
    mov eax, 0xE820
    mov dword [es:di+20], 1 ; Force ACPI 3.0 Extended attributes
    mov ecx, 24
    int 0x15
    jc .failed

    mov edx, 0x0534D4150
    cmp eax, edx
    jne .failed

    test ebx, ebx
    je .failed
    jmp .jin
.loop:
    mov eax, 0xE820
    mov dword [es:di+20], 1 ; Force ACPI 3.0 Extended attributes
    mov ecx, 24
    int 0x15
    jc .finish
    mov edx, 0x0534D4150
.jin:
    jcxz .skip
    cmp cl, 20
    jbe .noext
    test byte [es:di+20], 1
    je .skip
.noext:
    mov ecx, [es:di+8]
    or ecx, [es:di+12]
    jz .skip
    inc bp
    add di, 24
.skip:
    test ebx,ebx
    jne .loop
.finish:
    mov [ds:memInfoBlock], bp
    pop edx
    pop ecx
    pop ebx
    pop es
    clc
    ret
.failed:
    pop edx
    pop ecx
    pop ebx
    pop es
    stc
    ret

vesa_vbe_get_info:
    [bits 16]
    push bp
    mov bp, sp
    push es
    push di

    mov es, [bp+4]
    mov di, [bp+6]
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x4F
    jne .failed
    mov ax, 1
    jmp .after
.failed:
    xor ax, ax
.after:
    pop di
    pop es
    mov sp, bp
    pop bp
    ret

current_ideal_mode: dw 0x13
vesa_vbe_scan_mode:
    [bits 16]
    push bp
    mov bp, sp

    push dx ; Position: (BP-2)
    push bx ; Position: (BP-4)

    ; Save segment and offset of video mode address, located in
    ; VBEInfo block
    push dword [VBEInfo+0xE] ; Position: (BP-6)
.calc_ideal_pix_diff:
    mov ax, [bp+4] ; desired width
    mov dx, [bp+6] ; desired height
    mul dx ; Pix count = width*height
    shl edx, 16
    add eax, edx ; full 32 bit result of the multiplication
    push eax ; Save this for convenience
    ; Position: (BP-10)

    cmp eax, 320*200
    jl .pix_diff_lt
    ; x*y-320*200, if (x*y) > (320*200)
    mov edx, 320*200
    jmp .done_ideal_pix_diff
.pix_diff_lt:
    ; 320*200-x*y, if (x*y) < (320*200)
    mov edx, eax
    mov eax, 320*200
.done_ideal_pix_diff:
    sub eax, edx
    push eax ; Save pix_diff
    ; Position: (BP-14)
.calc_ideal_depth_diff:
    mov ax, [bp+8]
    cmp ax, 8
    jle .depth_diff_lte
    ; (d-8)*2, if d > 8
    sub ax, 8
    shl ax, 1
    jmp .done_ideal_depth_diff
.depth_diff_lte:
    ; 8-d, if d <= 8
    mov dx, ax
    mov ax, 8
    sub ax, dx
.done_ideal_depth_diff:
    push ax ; Save depth_diff
    ; Position: (BP-18)

    xor bx, bx ; Reset counter, prepare for loop
.loop:
    ; Obtain mode at index bx
    push ds
    push si
    mov ax, [bp-6] ; Segment of video mode address
    mov ds, ax
    mov si, [bp-8] ; Offset of address
    ; Ok, so after pulling my hair out debug this horrendous code, I've found out that there is a massive bug below,
    ; I'll comment the upsetting code out instead of deleting just for myself and you guys to see.
    
    ; The code below tries to increment the address from far pointer DS:SI, which points to a list of available VESA video modes.
    ; As you can see, it (past me, on a Wednesday night rushing to go to bed) tries to add the loop counter, BX, to the offset SI
    ; This is a fatal mistake, as the list of the video modes consists of 2-byte values, meaning that for this code to work, the
    ; intended behavior must be SI+2*BX.

    ; After fixing this, I have learned my lesson: maybe you shouldn't try to code past midnight and let yourself rest.

    ; Below is the upsetting code:
    ; add si, bx 
    
    mov ax, bx ; Loop counter: BX
    shl ax, 1 ; BX*2, as the VESA video mode list consists of 2-byte values for video modes
    add si, ax ; video_mode_addr[bx], or video_mode_addr+bx*2
    mov cx, [ds:si]
    pop si
    pop ds
    ; Break out of loop if mode is 0xFFFF
    cmp cx, 0xFFFF
    je .finish

    ; Get mode info
    push cx
    push word VBEModeBlock
    push ds
    call vesa_vbe_get_mode
    xor ax, 1
    jnz .continue

    ; Check for LFB support
    mov ax, word [VBEModeBlock]
    and ax, 0x90
    cmp ax, 0x90
    jne .continue

    ; Check if memory model is either 4 or 6 (packed or direct color mode)
    mov al, byte [VBEModeBlock+0x1B]
    cmp al, 4
    je .check_desired_res
.checkmmodel6:
    cmp al, 6
    jne .continue
.check_desired_res:
    ; Check desired width
    mov ax, [bp+4]
    cmp ax, word [VBEModeBlock+0x12]
    jne .calc_pix_diff
.check_desired_height:
    ; Check desired height
    mov ax, [bp+6]
    cmp ax, word [VBEModeBlock+0x14]
    jne .calc_pix_diff
.check_desired_bpp:
    ; Check desired color depth
    mov ax, [bp+8]
    cmp al, byte [VBEModeBlock+0x19]
    jne .calc_depth_diff
    mov [current_ideal_mode], cx ; Save mode[BX] to current_ideal_mode, and get outta here
    jmp .finish
; Here, The reason I put depth diff calcuation first, is to get the pix_diff saved
; after the depth diff, making it easier to compare later
.calc_depth_diff:
    mov ax, [bp+8] ; Desired depth
    xor ah, ah ; Just to be sure that the BPP is capped at 8 bits
    cmp al, byte [VBEModeBlock+0x19]
    jle .calc_depth_diff_lt
    sub al, byte [VBEModeBlock+0x19]
    shl al, 1
    jmp .done_depth_diff
.calc_depth_diff_lt:
    mov dx, ax
    mov al, byte [VBEModeBlock+0x19]
    sub al, dl
.done_depth_diff:
    push ax ; Position: (BP-20)
.calc_pix_diff:
    mov ax, word [VBEModeBlock+0x12] ; VBE MODE WIDTH
    mov dx, word [VBEModeBlock+0x14] ; VBE MODE HEIGHT
    mul dx
    shl edx, 16
    mov eax, edx
    cmp eax, dword [bp-10] ; Compare VBE Mode pix count with desired pix count
    jl .calc_pix_diff_lt
    mov edx, dword [bp-10]
    jmp .done_pix_diff
.calc_pix_diff_lt:
    mov edx, eax
    mov eax, [bp-10]
.done_pix_diff:
    sub eax, edx
    push eax ; Position: (BP-22)
.final_check:
    ; Check if either pix_diff < ideal_pix_diff
    ; or pix_diff == ideal_pix_diff and depth_diff < ideal_depth_diff

    mov eax, [bp-22] ; retrieve pix_diff, without popping
    cmp eax, dword [bp-14]
    jl .assign
    je .depth_check
    jmp .final_act
.depth_check:
    mov ax, [bp-20] ; retrieve depth_diff
    cmp ax, [bp-18]
    jge .final_act
.assign:
    mov ax, [bp-20]
    mov [bp-18], ax ; Assign depth_diff to ideal
    mov ax, [bp-22]
    mov [bp-14], ax ; Assign pix_diff to ideal
    mov [current_ideal_mode], cx; Video mode was saved in CX, so save that
.final_act:
    add sp, 6 ; Discard previously saved pix_diff and depth_diff
.continue:
    inc bx
    jmp .loop
.finish:
    add sp, 14 ; Discard previously saved vars
    pop bx
    pop dx

    mov ax, [current_ideal_mode] ; Return value will be the ideal mode

    mov sp, bp
    pop bp
    ret

vesa_vbe_get_mode:
    [bits 16]
    push bp
    mov bp, sp
    push es
    push di
    push cx

    mov es, [bp+4]
    mov di, [bp+6]
    mov cx, [bp+8]
    mov ax, 0x4F01
    int 0x10
    cmp ax, 0x4F
    jne .failed
    mov ax, 1
    jmp .after
.failed:
    xor ax, ax
.after:
    pop cx
    pop di
    pop es
    mov sp, bp
    pop bp
    ret

vesa_vbe_set_mode:
    [bits 16]
    push bp
    mov bp, sp
    push bx

    mov bx, [bp+4]
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x4F
    jne .failed
    mov ax, 1
    jmp .after
.failed:
    xor ax, ax
.after:
    pop bx
    mov sp, bp
    pop bp
    ret

load_gdt:
    [bits 16]
    lgdt [gdtr]
    ret

gdt:
    dq 0
    
    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 00001111b
    db 0

    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 00001111b
    db 0

gdtr: dw gdtr-gdt-1
     dd gdt



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bootDrive: db 0
partOffset: db 0
videoBlock: times 32 db 0
mbrBuffer: times 512 db 0
tempGlyphBuffer: times 4096 db 0
fontBlock:
    .height: db 0
    .width: db 0
    .glyph_count: dw 0
    .glyph_buffer: dd 0
memInfoBlock: times 4096 db 0
VBEInfo: times 512 db 0
VBEModeBlock: times 256 db 0