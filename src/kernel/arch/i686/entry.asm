bits 32

%define KRNL_PMEM 0x100000

%define PG_PRESENT (1<<0)
%define PG_RW (1<<1)

section .entry

extern kstart
extern __start
extern __end

global id_start

id_start:
    ; In the bootloader, to actually execute the kernel,
    ; we use: kinit(param)

    ; Which in i386 ASM, it translates to:
    ; push param
    ; call kinit

    ; Stack:
    ; | ESP   | ESP + 4 |
    ; | kinit | param   |

    ; Therefore, in order to get our needed param as a callee
    ; We will save (ESP+4) to somewhere safe:

    mov esi, [esp+4] ; We save the bootloader struct to a safe place
    mov [(boot_info_ptr-0xC0000000)], esi

    ; Higher half map from 0x0 -> 0xC0000000
    ; Also, for a smoother transition without the kernel
    ; foaming of page faults, we should ID-map the kernel
    ; then once we got to the higher-half, we can just
    ; unmap the ID-mapped kernel base.

    ; Means we will fill up both entry 0 and 768 
    ; in the page directory.

    lea eax, [(f4m_pt-0xC0000000)]
    or eax, PG_PRESENT | PG_RW
    mov [(page_dir-0xC0000000)], eax ; ID-map the first 4M of memory for safe transition
    mov [(page_dir-0xC0000000)+768*4], eax ; Our main course - the higher half map


    xor ecx, ecx
.id_map_f4m:
    mov eax, 0 ; We start from the address 0x0
    mov edx, ecx
    shl edx, 12
    add eax, edx
    or eax, (PG_PRESENT | PG_RW)
    mov [(f4m_pt-0xC0000000)+ecx*4], eax
    inc ecx
    cmp ecx, 1024
    jb .id_map_f4m

    lea eax, [(page_dir-0xC0000000)] ; Load page directory
    mov cr3, eax

    cli
    mov eax, cr0
    or eax, (1<<31) | (1<<16)
    mov cr0, eax ; Enable paging

    jmp 0x08:post_paging_setup

;;;;
;;;; From this part onwards, every variable in
;;;; the .data, .bss, and .rodata area are in higher half.
;;;; To get their physical addresses, subtract it by 0xC0000000.

section .text align=4096

post_paging_setup:
    ; Stack setup
    mov esp, stack_top

    mov ebx, [boot_info_ptr]
    ; Save params passed by bootloader
    mov ecx, 32
    mov edi, fb_block
    mov esi, [ebx+0x0]
    rep movsb

    mov ecx, 8
    mov edi, font_block
    mov esi, [ebx+0x8]
    rep movsb

    mov ecx, 16
    mov edi, sdp_block
    mov esi, [ebx+0x4]
    rep movsb

    mov ecx, 4096
    mov edi, glyph_buffer
    mov esi, [font_block+0x4]
    rep movsb

    mov dword [boot_info_block+0x0], fb_block
    mov dword [boot_info_block+0x4], font_block
    mov dword [boot_info_block+0x8], sdp_block

    mov eax, [ebx+0xC]
    mov dword [boot_info_block+0xC], eax

    mov eax, [ebx+0x10]
    mov dword [boot_info_block+0x10], eax

    mov dword [font_block+0x4], glyph_buffer

    ; Load previously saved params
    mov esi, boot_info_block

    ; Pass the info for the kernel to do its thing
    push esi
    call kstart
    cli
    hlt

section .bss align=4096

glyph_buffer: resb 4096

align 4096
page_dir: resd 1024
f4m_pt: resd 1024 ; First 4M

section .data align=4096

boot_info_ptr: dd 0
boot_info_block: times 20 db 0
fb_block: times 32 db 0
font_block: times 8 db 0
sdp_block: times 16 db 0

section .stack align=16

stack_bot:
    resb 0x8000
stack_top: