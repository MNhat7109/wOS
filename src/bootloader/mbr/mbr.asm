org 0x600
[bits 16]

reloc:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x600

    mov cx, 256
    mov di, 0x600
    mov si, 0x7c00
    repe movsw
    jmp 0:lo_main

lo_main:
    ; Load stage2 of MBR. 440 bytes of code is just too small to fit anything inside.
    mov [bootDrive], dl

    mov dl, [bootDrive]
    call disk_check_ext
    test al, al
    jnz halt

    mov dl, [bootDrive]
    mov eax, 1
    mov di, 0x800
    xor ecx, ecx
    mov bx, 1
    call disk_read
    jc halt
    jmp 0:lo_main_2

halt:
    mov si, error_str
    call print
    cli
    hlt

%include "../disk.inc"
%include "../print.inc"
error_str: db 'Stage 2 not loaded'

times 218-($-$$) nop

disk_tstamp times 8 db 0
bootDrive: db 0
part_offset: dw 0

times 0x1b4-($-$$) nop

UID times 10 db 0
P1:
    .attr: db 0x80
    .chs_start: db 0x20, 0x21,0x00
    .type: db 0x0c
    .chs_end: db 0x14, 0x05,0x04
    .lba_start: dd 2048
    .count: dd 67584
P2 times 16 db 0
P3 times 16 db 0
P4 times 16 db 0

dw 0xaa55

lo_main_2:
    ; Ckeck for active partitions
    mov si, 0x7BE ; Address pointer to First PT Entry (0x600:org + 0x1BE: PTE)
    mov cx, 4 ; Partition count
.partition_check:
    mov ax, [si]
    test ax, 0x80 ; Is the partition active?
    jnz .done

    add si, 16 ; Next partition entry
    dec cx
    jnz .partition_check ; If CX is not zero, loop

.error:
    ; The bootable partition cannot be found. Halt.
    mov si, PT_error_str
    call print
    cli
    hlt

.done:
    ; Done. Get the partition offset, and leave.
    mov word [part_offset], si

next:
    ; Read the partition to 0x7c00, and send out the partition offset
    ; and the boot drive.
    mov dl, [bootDrive]
    mov si, [part_offset]
    mov bx, 1
    mov di, 0x7c00
    mov eax, [si+8] ; get the LBA of partition
    xor ecx, ecx
    stc
    call disk_read
    jc .halt
    
    xor bx, bx
    mov bx, [part_offset]
    xor dx, dx
    mov dl, [bootDrive]

    jmp 0:0x7c00

.halt:
    mov si, read_error_str
    call print
    cli
    hlt

read_error_str: db 'The partition cannot be read.',0
PT_error_str: db 'The bootable partition cannot be found.',0
times 1024-($-$$) db 0