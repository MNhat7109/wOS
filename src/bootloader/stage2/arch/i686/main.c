#include "stdint.h"
#include "interrupt/interrupt.h"
#include "interrupt/pic.h"
#include "timer/pit.h"
#include "disk/disk.h"
#include "fat/fat.h"
#include "elf/elf.h"
#include "vesa/vesa.h"
#include "vesa/font.h"
#include "rsdp/rsdp.h"

typedef struct
{
    u32 entries_count;
    struct
    {
        u64 base, length;
        u32 type;
        u32 acpi;
    } __attribute__((packed)) regions[];
} __attribute__((packed)) memory_info_t;

struct boot_info_t
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    system_desc_ptr_t* sdp;
    font_t* font_out;
    memory_info_t* mem_map;
} __attribute__((packed)) boot_info;

typedef void (*kernel_func_t)(struct boot_info_t* boot_info);

memory_info_t* mmap;

void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset, framebuffer_t* framebuffer, memory_info_t* memInfo)
{
    
end:    
    for (;;);
}