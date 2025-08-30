bits 32

%define KRNL_PMEM 0x100000

%define PG_PRESENT (1<<0)
%define PG_RW (1<<1)

section .entry

extern kstart

global pre_paging_start
pre_paging_start:
    pop esi ; We save the bootloader struct to a save place
    mov [boot_info_ptr], esi
    
    ; Higher half map from 0x100000 -> 0xC0000000
    ; Also, for a smoother transition without the kernel
    ; foaming of page faults, we should ID-map the kernel
    ; then once we got to the higher-half, we can just
    ; unmap the ID-mapped kernel base.

    ; Means we will fill up both entry 0 and 768 
    ; in the page directory.

    lea eax, [f4m_pt]
    or eax, PG_PRESENT | PG_RW
    mov [page_dir], eax ; ID-map the first 4M of memory for safe transition
    
    lea eax, [kernel_pt] ; Can be mov eax, kernel_pt
    or eax, PG_PRESENT | PG_RW
    mov [page_dir+768*4], eax ; Our main course - the higher half map

    ; Map first 4M of kernel
    xor ecx, ecx
.map_krnl_page:
    mov eax, KRNL_PMEM
    mov edx, ecx
    shl edx, 12
    add eax, edx
    or eax, PG_PRESENT | PG_RW
    mov [kernel_pt+ecx*4], eax
    inc ecx
    cmp ecx, 1024 ; Do we reach enough pages?
    jb .map_krnl_page

    xor ecx, ecx
.id_map_f4m:
    mov eax, 0 ; We start from the address 0x0
    mov edx, ecx
    shl edx, 12
    add eax, edx
    or eax, PG_PRESENT | PG_RW
    mov [f4m_pt+ecx*4], eax
    inc ecx
    cmp ecx, 1024
    jb .id_map_f4m

    lea eax, [page_dir] ; Load page directory
    mov cr3, eax

    cli
    mov eax, cr0
    or eax, (1<<31)
    mov cr0, eax ; Enable paging
    jmp 0x08:post_paging_setup


section .text align=4096
global post_paging_setup

post_paging_setup:
    ; Stack setup
    mov esp, stack_top

    ; Load previously saved params
    mov esi, [boot_info_ptr]

    ; Pass the info for the kernel to do its thing
    push esi
    call kstart
    cli
    hlt

section .bss align=4096

align 4096
page_dir: resd 1024

kernel_pt: resd 1024
f4m_pt: resd 1024 ; First 4M

section .data align=4096

boot_info_ptr: dd 0

section .stack align=16

stack_bot:
    resb 0x4000
stack_top: