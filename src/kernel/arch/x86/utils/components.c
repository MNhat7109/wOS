#include <arch/x86/utils/kernel/kernel.h>
#include <arch/x86/utils/boot.h>
#include <arch/x86/common/gdt.h>
#include <arch/x86/common/idt.h>

#include <kutils/mmu/heap.h>

#include <libk/interrupt/isr.h>
#include <libk/mmu/mmu.h>
#include <libk/mmu/page_allocator.h>
#include <libk/bitmap/bitmap.h>
#include <libk/video/video.h>
#include <libk/stdio.h>

#include <devices/driver.h>
#include <devices/driver_defs.h>

#include <devices/acpi/acpi_defs.h>
#include <devices/acpi/acpi.h>
#include <devices/cpu/cpu.h>

static struct
{
    gdt_entry_t kernel_gdt[6];
    idt_entry_t kernel_idt[256];
    struct generic_driver_tree_node_t* acpi_driver_node,
    *cpu_driver_node, *pci_driver_node;
} kernel_data;

gdt_entry_t kernel_gdt[6];
idt_entry_t kernel_idt[256];
struct generic_driver_tree_node_t* acpi_driver_node=NULL;

void kernel_prepare_gdt()
{
    x86_GDT_init(&kernel_data.kernel_gdt);

    // Create the entries themselves
    // We start at the second entry, as the first one has been a
    // NULL descriptor

    // Two for kernel code and data,
    // and two for user code and data

    x86_GDT_set_attr(
        1, 
        0, 
        0xFFFFF,
        (GDT_ACCESS_PVL_KRNL | 
        GDT_ACCESS_CODE_DATA_SEG | 
        GDT_ACCESS_EXECUTABLE | 
        GDT_ACCESS_READ_WRITE),    
        (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE)
    ); // Kernel code segment

    x86_GDT_set_attr(
        1, 
        0, 
        0xFFFFF,
        (GDT_ACCESS_PVL_KRNL | 
        GDT_ACCESS_CODE_DATA_SEG |  
        GDT_ACCESS_READ_WRITE),    
        (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE)
    ); // Kernel data segment

    x86_GDT_set_attr(
        2, 
        0, 
        0xFFFFF,
        (GDT_ACCESS_PVL_USER | 
        GDT_ACCESS_CODE_DATA_SEG | 
        GDT_ACCESS_EXECUTABLE | 
        GDT_ACCESS_READ_WRITE),    
        (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE)
    ); // User code segment

    x86_GDT_set_attr(
        3, 
        0, 
        0xFFFFF,
        (GDT_ACCESS_PVL_USER | 
        GDT_ACCESS_CODE_DATA_SEG |  
        GDT_ACCESS_READ_WRITE),    
        (GDT_FLAG_PAGE_GRAN | GDT_FLAG_SIZE)
    ); // User data segment

    for (int i=1;i<5;i++) x86_GDT_mark_present(i);

    // Finally, let the CPU sip that thing up
    // And reload the segments
    x86_GDT_load_entries(sizeof(kernel_data.kernel_gdt),KERNEL_CODE_SEG, KERNEL_DATA_SEG);
}

void kernel_prepare_interrupts()
{
    // IDT setup
    x86_IDT_init(&kernel_data.kernel_idt);
    x86_IDT_load_entries(sizeof(kernel_data.kernel_idt));

    // ISR
    ISR_init();
}

void kernel_prepare_mmu(boot_info_t* info)
{
    void* bmp_buffer = BMP_BUFFER_ADDR; 
    mmu_init(info, bmp_buffer, false);

    // Map anything that's kernel related

    // ACPI's RSDP (XSDP)
    usize sdp_page_count = mmu_byte_to_page_count(sizeof(system_desc_ptr_t));
    page_alloc_lockn(info->sdp, sdp_page_count);
    mmu_mmapn((usize)info->sdp, 0, sdp_page_count, MMU_PT_FLAG_READ_WRITE);

    // The E820 memory map
    memory_info_t* memmap = (memory_info_t*)info->mem_map;
    usize mem_map_page_cnt = mmu_byte_to_page_count(sizeof(memory_info_t)
    +sizeof(memory_region_t)*memmap->entries_count);
    page_alloc_lockn(info->mem_map, mem_map_page_cnt);
    mmu_mmapn(info->mem_map, 0, mem_map_page_cnt, MMU_PT_FLAG_READ_WRITE);

    // The whole kernel (including the bss)
    usize offset = ((usize)&__start) - ((usize)&__low_start);
    usize krnl_size = ((usize)&__end) - ((usize)&__start);
    usize krnl_page_cnt = mmu_byte_to_page_count(krnl_size);
    page_alloc_lockn(&__low_start, krnl_page_cnt);
    mmu_mmapn(((usize)&__low_start), offset, krnl_page_cnt, MMU_PT_FLAG_READ_WRITE);

    // Remap the .text and .rodata to Read-Only to improve security

    usize text_phys = ((usize)&__text_start) - offset;
    usize text_size = ((usize)&__text_end) - ((usize)&__text_start);
    usize text_page_cnt = mmu_byte_to_page_count(text_size);
    mmu_mmapn(text_phys, offset, text_page_cnt, 0);

    usize rodata_phys = ((usize)&__rodata_start) - offset;
    usize rodata_size = ((usize)&__rodata_end) - ((usize)&__rodata_start);
    usize rodata_page_cnt = mmu_byte_to_page_count(rodata_size);
    mmu_mmapn(rodata_phys, offset, rodata_page_cnt, MMU_PT_FLAG_NX);

    // The framebuffer
    framebuffer_t* fb_ptr = (framebuffer_t*)info->framebuffer;
    usize fb_page_count = mmu_byte_to_page_count(fb_ptr->size);
    page_alloc_lockn(fb_ptr->base, fb_page_count);
    mmu_mmapn(fb_ptr->base, 0, fb_page_count, 
        MMU_PT_FLAG_READ_WRITE|MMU_PT_FLAG_CACHE_DISABLE);

    // The font glyph
    font_t* font_ptr = (font_t*)info->font_out;
    u8 char_size = font_ptr->height*((font_ptr->width+7)>>3);
    usize glyph_page_count = mmu_byte_to_page_count(font_ptr->glyph_count*char_size);
    page_alloc_lockn(font_ptr->glyph, glyph_page_count);
    mmu_mmapn(font_ptr->glyph, 0, glyph_page_count, MMU_PT_FLAG_NX);
    
    // The bitmap allocator buffer
    bitmap_t* page_alloc_bmp = page_get_bitmap();
    usize bmp_size_in_bytes = page_alloc_bmp->size;
    usize bmp_page_count = mmu_byte_to_page_count(bmp_size_in_bytes);
    page_alloc_lockn(page_alloc_bmp->buffer, bmp_page_count);
    mmu_mmapn(page_alloc_bmp->buffer, 0, bmp_page_count, MMU_PT_FLAG_READ_WRITE);

    // Enable paging (2nd time, so that we can reinforce the new page mapping
    // and new features like PAE and NX)
    mmu_enable();

    // Set up heap
    mmu_init_heap(KERNEL_HEAP_ADDR, KERNEL_MAX_HEAP_SIZE/MMU_PAGE_SIZE);
}

void kernel_prepare_drivers()
{
    // ACPI
    kernel_data.acpi_driver_node
    = driver_add_to_tree(&
        driver_forest,
        NULL,
        DRIVER_ID_TYPE_INTERNAL,
        DRIVER_BUS_TYPE_STANDALONE,
        100,
        DRIVER_MODE_KRNL
    );
    driver_set_id_data(kernel_data.acpi_driver_node, "GENERIC_ACPI_ROOT_DEV");
    driver_load_ops(kernel_data.acpi_driver_node, acpi_get_driver_ops);
    
    // CPU
    kernel_data.cpu_driver_node
    = driver_add_to_tree(
        &driver_forest,
        NULL,
        DRIVER_ID_TYPE_INTERNAL,
        DRIVER_BUS_TYPE_STANDALONE,
        100,
        DRIVER_MODE_KRNL
    );
    driver_set_id_data(kernel_data.cpu_driver_node, "GENERIC_CPU_ROOT_DEV");
    driver_load_ops(kernel_data.cpu_driver_node, cpu_get_driver_ops);
}

void kernel_prepare_root_dev(boot_info_t* info)
{
    // ACPI
    
    acpi_param_t acpi_parms;
    acpi_parms.sys_desc_ptr = info->sdp;
    kernel_data.acpi_driver_node->additionals = (void*)&acpi_parms;
    if (!driver_run(kernel_data.acpi_driver_node))
    {
        kprintf("Kernel: Failed to start ACPI driver\n");
        driver_remove_from_tree(
            &driver_forest,
            kernel_data.acpi_driver_node
        );
    }

    // CPU

    if (!driver_run(kernel_data.cpu_driver_node))
    {
        kprintf("Kernel: Failed to start CPU driver\n");
        driver_remove_from_tree(
            &driver_forest,
            kernel_data.cpu_driver_node
        );
    }

    // PCI
}