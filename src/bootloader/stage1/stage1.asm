org 0x1000
[bits 16]

jmp short _start
nop

; Basically everything about the BIOS param_block
param_block:
    .oem_id: dq 0
    .bytes_per_sector: dw 0
    .sectors_per_cluster: db 0
    .reserved_sectors: dw 0
    .fat_count: db 0
    .root_dir_entries: dw 0 ; In FAT32, this value must be zero
    .total_sectors: dw 0 ; In FAT32, this is also zero because of
                         ; sector count being larger than 65535.
    .media_desc_type: db 0
    .sectors_per_fat: dw 0 ; Use this for FAT1x (12 or 16) only.
                           ; Leave this zero.
    .sectors_per_track: dw 0
    .heads: dw 0
    .hidden_sectors: dd 0
    .large_sector_count: dd 0

extended_bootrec:
    .sectors_per_fat: dd 0
    .flags: dw 0
    .fat_version: dw 0
    .root_cluster: dd 0
    .fs_info_sectors: dw 0
    .backup_sector: dw 0
    .reserved times 12 db 0
    .drive_number: db 0
    .flags_nt: db 0
    .signature: db 0
    .volume_id: dq 0
    .volume_label times 11 db 0
    .system_id: dq 0


_start:
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x1000

.reloc:
    push si ; Parted's MBR saved the MBR partition offset at register SI, so we save it
    mov cx, 256
    mov di, 0x1000
    mov si, 0x7c00
    rep movsw
    mov ax, .pre_main
    jmp 0:.pre_main

.pre_main:
    sti
    push es
    push word .main
    retf

.main:
    pop si ; Retrieve the offset back from stack
    ; Read from disk first
    mov [extended_bootrec.drive_number], dl
    mov [part_offset], si

    mov dl, [extended_bootrec.drive_number]
    mov bx, 2
    mov di, 0x1200
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
    mov si, [current_cluster]
    call cluster_to_lba ; LBA is saved in EAX
    xor bh, bh
    mov bl, [param_block.sectors_per_cluster]
    mov di, buffer
    xor ecx, ecx
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
    mov di, stage2_addr

.load_file:
    mov si, [current_cluster] ; Load current cluster
    call cluster_to_lba ; LBA is conveniently saved in EAX
    mov dl, [extended_bootrec.drive_number]
    xor bh, bh
    mov bl, [param_block.sectors_per_cluster]
    xor cx, cx
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
    mov si, stage2_addr
    jmp si

.halt:
    cli
    hlt


%include "../print.inc"

; Input: SI = Cluster number
; Output: EAX = LBA of cluster

cluster_to_lba:
    push edx
    push ebx
    push di

    mov di, [part_offset]
    mov eax, [di+8] ; First sector of partition
    xor ebx, ebx
    mov bx, [param_block.reserved_sectors] ; Reserved sectors
    add eax, ebx
    push eax ; EAX = MBR_LBA_START + R
    xor eax, eax
    mov al, [param_block.fat_count]
    mov ebx, [extended_bootrec.sectors_per_fat]
    mul ebx ; FAT*SPF
    mov ebx, eax
    pop eax ; EAX = MBR_LBA_START + R
    add eax, ebx
    push eax ; EAX = MBR_LBA_START + R + FAT*SPF (This is the first sector of FAT32's data region)
    mov ax, [current_cluster]
    sub ax, 2
    xor bh, bh
    mov bl, [param_block.sectors_per_cluster] ; Sectors per cluster
    mul bx ; (cluster-2)*SPC
    shl edx, 16
    add eax, edx
    mov ebx, eax
    pop eax ; EAX = MBR_LBA_START + R + FAT*SPF
    add eax, ebx ; EAX = MBR_LBA_START + R + FAT*SPF + (cluster-2)*SPC

.done:
    pop di
    pop ebx
    pop edx
    ret

; FIXME:
next_cluster:
    push edx
    push ecx
    push ebx
    push di
    
.work:
    xor eax, eax
    shl edx, 2 ; Offset = Cluster*4 bytes (Hence FAT32!)
    mov ax,dx ; Divide 32 bit
    shr edx, 16 ; High word in DX, Low in AX
    mov cx, 512 ; Divide by bytes per sector
    div cx 
    ; AX = Quotient (Sector position in FAT table)
    ; DX = Remainder (Index in that one sector)
    push dx
    ; EAX = FAT_SECTOR_POS

    mov dl, [extended_bootrec.drive_number]
    mov si, [part_offset]
    add eax, [si+8] ; EAX = LBA_START+FAT_SECTOR_POS
    xor ebx, ebx
    mov bx, [param_block.reserved_sectors]
    add eax, ebx ; EAX = LBA_START+R+FAT_SECTOR_POS
    xor ecx, ecx
    mov di, FAT
    mov bx, 1
    stc
    call disk_read

    pop dx
    add di, dx
    mov eax, dword [es:di] ; EAX = Cluster = FAT[idx]
    and eax, 0x0FFFFFFF

.done:
    pop di
    pop ebx
    pop ecx
    pop edx
    ret

stage2_addr: equ 0x1800
current_cluster: dd 0
nfe_str: db 'File not found', 0x0a,0x0d,0
hello_str: db 'Hi guys!', 0x0a, 0x0d,0
file_name: db 'BOOT    BIN'
times 1536-($-$$) db 0
FAT:
times 2048-($-$$) db 0
buffer: