bits 32

global mmu_flush_tlb
global mmu_load_address_space_i686
global mmu_enable_paging
global mmu_enable_pse

; /**
; *
; * @brief Invalidate one TLB entry.
; *
; * @param vaddr
; *    Virtual address of the page to invalidate.
; *    Passed in: [esp+4]
; * @clobbers eax
; */
mmu_flush_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    invlpg [eax]

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Load physical address space to CR3.
; *
; * @param paddr
; *    Physical address of the address space to load.
; *    Passed in: [esp+4]
; * @clobbers eax
; */
mmu_load_address_space_i686:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

; /**
; *
; * @brief Enable paging mode.
; *
; * @clobbers eax
; */
mmu_enable_paging:
    mov eax, cr0
    or eax, (1<<31)
    mov cr0, eax
    ret

; /**
; *
; * @brief Enable PSE for huge 4MiB pages.
; *
; * @clobbers eax
; */
mmu_enable_pse:
    mov eax, cr4
    or eax, (1<<4)
    mov cr4, eax
    ret