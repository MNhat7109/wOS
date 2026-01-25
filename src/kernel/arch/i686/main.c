#include <stdint.h>
#include <kernel/arch/i686/cpuid.h>
#include <kernel/mmu.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu_frame.h>
#include <kernel/debug.h>

#define MODULE_KRNL "KMAIN"

extern u8 __low_start;
extern u8 __entry_start;
extern u8 __start;
extern u8 __text_start;
extern u8 __text_end;
extern u8 __data_start;
extern u8 __rodata_start;
extern u8 __rodata_end;
extern u8 __bss_start;
extern u8 __bss_end;
extern u8 __stack_start;
extern u8 __stack_end;
extern u8 __end;

typedef struct framebuffer_t framebuffer_t;
typedef struct system_desc_ptr_t system_desc_ptr_t;
typedef struct font_t font_t;

typedef struct boot_info_t
{
    framebuffer_t* framebuffer;
    system_desc_ptr_t* sdp;
    font_t* font_out;
    memory_info_t* mem_map;
} boot_info_t;

typedef enum
{
    PARAM_PAE_ON = (1<<0),
    PARAM_NX_ON = (1<<1),
    PARAM_PSE_ON = (1<<2),
} optional_param_t;

static struct
{
    u8 kmemlock_done;
    u8 kmemmap_done;
    usize kernel_size;
    usize kernel_phys;
    // TODO: every component's status
} kernel_data;

void stdio_register_putc(void (*putc_op)(char ch));
void stdio_register_puts(void (*puts_op)(const char* str));

void debug_console_init();
void debug_console_putch(char ch);
void debug_console_write(const char* str);

void mmu_arch_init(uptr first_free_page, u32 optional_features);

int kmemlock(memory_info_t* mem_info)
{
    if (kernel_data.kmemlock_done) goto done;
    int status;
    
    usize offset = ((usize)&__start)-((usize)&__low_start);
    kdebugf(DEBUG_INFO, "KMAIN", "End: 0x%x, offset=0x%x\n", &__end, offset);
    status = mmu_init(((uptr)&__end), mem_info);
    if (status < 0) return status;

    // Lock necessary components so that it won't page fault when 
    // enabling paging for a second time
    
    kernel_data.kernel_size = ((usize)&__end) - ((usize)&__start);
    usize kernel_page_count = mmu_byte_to_4k_pages(kernel_data.kernel_size);
    kernel_data.kernel_phys = mmu_vtop((vaddr_t)&__start);
    mmu_frame_reserve_n((uptr)kernel_data.kernel_phys, kernel_page_count);

done:
    return 0;
}

int kmemmap()
{
    if (kernel_data.kmemmap_done) goto done;

    // Request one page to store page directories on
    uptr first_free_page = mmu_frame_next();

    kdebugf(DEBUG_INFO, "MMU", "First free page at: 0x%x\n", first_free_page);

    // Init arch function to do arch-specific things
    u32 additional = PARAM_PSE_ON; // PAE: 0, NX: 0, PSE: 1
    mmu_arch_init(first_free_page, additional); // Init paging with additional features
        
    
    usize kernel_page_count = mmu_byte_to_4k_pages(kernel_data.kernel_size);

    // Map everything else in the kernel as RW
    mmu_mmapn(kernel_data.kernel_phys, kernel_page_count, MMU_PG_ATTR_RW, 0);
    
    // Remap .text and .rodata to RO
    usize text_size = ((usize)&__text_end) - ((usize)&__text_start);
    usize text_page_count = mmu_byte_to_4k_pages(text_size);
    paddr_t text_phys_start = mmu_vtop((vaddr_t)&__text_start);
    mmu_mmapn(text_phys_start, text_page_count, 0, 0);

    usize rodata_size = ((usize)&__rodata_end) - ((usize)&__rodata_start);
    usize rodata_page_count = mmu_byte_to_4k_pages(rodata_size);
    paddr_t rodata_phys_start = mmu_vtop((vaddr_t)&__rodata_start);
    mmu_mmapn(rodata_phys_start, rodata_page_count, 0, 0);

    // Now after everything has been mapped, wait for other components to initialize and
    // and map its region, then we can enable paging
    kernel_data.kmemmap_done = 1;
done:
    return 0;
}

void kmemstart()
{
    u8 ok = 1;
    ok = ok && kernel_data.kmemmap_done;

    if (!ok) goto done;
    mmu_enable_features();

    u64 mem_size = mmu_get_total_size();
        u64 usable, hole, reserved, other;
    usable = mmu_get_zone_size(MMU_ZONE_FREE);
    reserved = mmu_get_zone_size(MMU_ZONE_HW_RESERVED);
    hole = mmu_get_zone_size(MMU_ZONE_HOLE);
    other= mmu_get_zone_size(MMU_ZONE_OTHER);

    kdebugf(DEBUG_INFO, MODULE_KRNL, "MMU initialized successfully\n");
    kdebugf(DEBUG_INFO, MODULE_KRNL, "Additional info:\n"
        "\tMMU components located at 0x%x\n"
        "\tTotal addressable memory: %llu bytes\n"
        "\tUsable memory: %llu bytes\n"
        "\tBIOS-reserved memory: %llu bytes\n"
        "\tHole: %llu bytes\n"
        "\tOther : %llu bytes\n"
        "\tKernel total size: %u bytes\n",
        &__end,
        mem_size,
        usable,
        reserved,
        hole,
        other,
        kernel_data.kernel_size
    );

done:
    return;
}

void kstart(boot_info_t* boot_inf)
{
    debug_console_init();
    debug_console_write("\x1B[2J\x1B[H"); // Clear the serial output, set cursor to beginning
    debug_console_write("Kernel: If you can see this message on serial, early logging is working\n");

    // Register stdio ops
    stdio_register_putc(debug_console_putch);
    stdio_register_puts(debug_console_write);

    kdebugf(DEBUG_INFO, "KMAIN", "Leveled logs arrived\n");

    cpuid_check();
    
    int status;
    status = kmemlock(boot_inf->mem_map);
    if (status < 0) goto end;

    status = kmemmap();
    if (status < 0) goto end;

    kmemstart();
    
    // if (!boot_prepare(boot_inf))
    // {
    //     goto end;
    // }

    // boot_prepare_acpi();

    // peripherals_init();

    // HAL_init_essentials();
    
    // // Set up timer for sleep()
    // ktime_init();
    
    // // Set up storage
    // disk_init();
    
    // // Set up scheduling for multitasking
    // // scheduling_init();
    
end:    for (;;);
}