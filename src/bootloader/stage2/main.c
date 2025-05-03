#include "stdint.h"
#include "stdio.h"
#include "interrupt/interrupt.h"
#include "timer/pit.h"
#include "disk/disk.h"
#include "fat/fat.h"

void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset)
{
    // Save params,
    // just in case of a fucking tragedy where I "accidentally" insert the wrong value to disk.
    void* offset = partitionOffset;
    interrupt_init();
    PIT_init(2000);
    disk_init(0, offset); // Mount our drive and partition
    // Welp, I guess we can write a FAT32 filesystem driver now, eh?
    if (!FAT_init())
    {
        kprintf("FAT init failed\n");
        goto end;
    }
    kprintf("Hello from stage2!\n");
end:    
    for (;;);
}