org 0x7c00
[bits 16]

jmp short _start
nop

; Basically everything about the BIOS param_block
param_block:
    .oem_id: db 'mkfs.fat'
    .bytes_per_sector: dw 512
    .sectors_per_cluster: db 1
    .reserved_sectors: dw 32
    .fat_count: db 2
    .root_dir_entries: dw 0 ; In FAT32, this value must be zero
    .total_sectors: dw 0 ; In FAT32, this is also zero because of
                         ; sector count being larger than 65535.
    .media_desc_type: db 0xF8
    .sectors_per_fat: dw 0 ; Use this for FAT1x (12 or 16) only.
                           ; Leave this zero.
    .sectors_per_track: dw 32 ; gonna leave this blank. CHS is long gone.
    .heads: dw 8 ; Same reason.
    .hidden_sectors: dd 0 ; Nothing to hide.
    .large_sector_count: dd 67584 ; That's 33 MiB.

extended_bootrec:
    .sectors_per_fat: dd 520
    .flags: dw 0
    .fat_version: dw 0
    .root_cluster: dd 2
    .fs_info_sectors: dw 1
    .backup_sector: dw 6
    .reserved times 12 db 0
    .drive_number: db 0x80 ; This number might not be used. 
                           ; We have MBR with partition entry(ies)
    .flags_nt: db 0
    .signature: db 0x28
    .volume_id: db 0xCA, 0xFE, 0xBA, 0xBE
    .volume_label: db 'W OS       '
    .system_id: db 'FAT32   '


_start:
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x7c00

    sti
    push es
    push word .main
    retf

.main:
    ; Read from disk first
    mov [extended_bootrec.drive_number], dl
    mov [part_offset], bx

    mov dl, [extended_bootrec.drive_number]
    mov bx, 2
    mov di, 0x7e00
    mov si, [part_offset]
    mov eax, [si+8] ; LBA of partition
    add eax, 2 ; LBA 2048+LBA 2 in partition
    xor ecx, ecx
    stc
    call disk_read
    jnc .finaljmp

.halt:
    cli
    hlt

.finaljmp:
    jmp 0:stage1_1

part_offset: dw 0
%include "../disk.inc"

times 510-($-$$) db 0
dw 0xaa55

stage1_1:
    mov si, hello_str
    call print

    mov ax, [extended_bootrec.root_cluster]
    mov [current_cluster], ax

.loop_root_dir:
    mov dl, [extended_bootrec.drive_number]
    mov si, [part_offset]
    mov eax, [si+8]
    add ax, [current_cluster]
    add ax, 1070 ; Hard-coded value for the LBA.
    mov di, buffer
    xor ecx, ecx
    mov bl, [param_block.sectors_per_cluster]
    stc
    call disk_read

    xor ax, ax
    mov di, buffer
.search:
    mov bl, [di+11] ; Is it a Long File Name entry?
    cmp bl, 0x0F
    je .read_root_after

    cmp bl, 0
    je .next_cluster

.read_root:
    mov si, file_name
    push di
    mov cx, 11
    repe cmpsb
    pop di

    je .found

.read_root_after:
    add di, 0x20
    jmp .search

    ; next cluster please
.next_cluster:
    mov edx, [current_cluster]
    call next_cluster
    cmp eax, 0x0FFFFFF8
    jae .file_not_found

    mov [current_cluster], eax
    jmp .loop_root_dir

.file_not_found:
    mov si, nfe_str
    call print

.found:
    mov ax, [di+20] ; Take cluster of file
    shl ax, 16
    mov ax, [di+26]
    mov [current_cluster], eax
    mov di, 0xA00

.load_file:
    mov si, [part_offset]
    add eax, [si+8]
    add eax, 1070
    xor ecx, ecx
    mov dl, [extended_bootrec.drive_number]
    mov bx, 1
    stc
    call disk_read
    mov edx, [current_cluster]
    call next_cluster
    cmp eax, 0x0FFFFFF8
    jae .finish
    
    mov [current_cluster], eax
    add di, 0x200
    jmp .load_file

.finish:
    xor ax,ax
    mov ds, ax
    mov es, ax
    mov dl, [extended_bootrec.drive_number]
    mov bx, [part_offset]
    mov si, 0xA00
    jmp si

.halt:
    cli
    hlt


%include "../print.inc"

next_cluster:
    push edx
    push ecx
    push bx
    push di
    
.work:
    shl edx, 2
    mov ax,dx
    shr edx, 16
    mov cx, 512
    div cx
    push dx

    mov dl, [extended_bootrec.drive_number]
    mov si, [part_offset]
    add eax, [si+8]
    add ax, [param_block.reserved_sectors]
    xor ecx, ecx
    mov di, FAT
    mov bx, 1
    stc
    call disk_read

    pop dx
    add di, dx
    mov eax, [es:di]

.done:
    pop di
    pop bx
    pop ecx
    pop edx
    ret


current_cluster: dd 0
nfe_str: db 'File not found', 0x0a,0x0d,0
hello_str: db 'Hi guys!', 0x0a, 0x0d,0
file_name: db 'BOOT    BIN'
times 1536-($-$$) db 0
FAT:
times 2048-($-$$) db 0
buffer: