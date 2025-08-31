#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

#define BMP_BUFFER_ADDR ((void*)0x200000)
#define KERNEL_HEAP_ADDR ((void*)0xC0400000)
#define USER_MODE_START 0x400000

#define KERNEL_MAX_HEAP_SIZE (8*1024*1024)

// To be safe, we will declare everything we've
// previously mentioned in the linker script

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

struct boot_info_t;
typedef struct boot_info_t boot_info_t;

extern boot_info_t* bootloader_info;
void kernel_prepare(boot_info_t* info);
void kernel_prepare_gdt();
void kernel_prepare_interrupts();
void kernel_prepare_mmu(boot_info_t* info);
void kernel_prepare_drivers();
void kernel_prepare_root_dev(boot_info_t* info);