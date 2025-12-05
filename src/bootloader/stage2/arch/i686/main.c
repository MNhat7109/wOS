#include "../../stdint.h"
#include "../../boot_info.h"
#include "../../drivers/console.h"

boot_info_t boot_info;
typedef void (*kernel_func_t)(boot_info_t* boot_info);

void __attribute__((cdecl)) start(u16 bootDrive, 
    void* partitionOffset, 
    framebuffer_t* framebuffer, 
    font_t* font, 
    memory_info_t* memInfo
)
{
    boot_info = (boot_info_t){
        .framebuffer = framebuffer,
        .font_out = font,
        .mem_map = memInfo,
        .partition_offset = partitionOffset,
        // TODO: ACPI
    };
    
    console_init(CONSOLE_MODE_BOTH, &boot_info);
end:    
    for (;;);
}