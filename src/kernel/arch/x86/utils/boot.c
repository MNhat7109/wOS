#include <stdbool.h>
#include <libk/string.h>
#include <libk/video/video.h>

#include <arch/x86/utils/boot.h>
#include <arch/x86/utils/kernel/kernel.h>

boot_info_t bootloader_info;

// The main kernel code.
// Here, we migrate every pointer we can find from the
// bootloader low memory's info to a safely mapped place
// in the kernel (which is here)

void kernel_prepare(boot_info_t* info)
{
    // Copy the content to a safe place
    memcpy(&bootloader_info, info, sizeof(boot_info_t));

    // Set up display
    video_init((framebuffer_t*)bootloader_info.framebuffer, 
    (font_t*)bootloader_info.font_out);

    // Set up GDT, IDT and ISRs
    kernel_prepare_gdt();
    kernel_prepare_interrupts();

    // Set up the MMU
    kernel_prepare_mmu(&bootloader_info);
    
    // Set up drivers
    kernel_prepare_drivers();
}