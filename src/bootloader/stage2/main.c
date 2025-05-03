#include "stdint.h"
#include "stdio.h"
#include "interrupt/interrupt.h"
#include "timer/pit.h"
#include "disk/disk.h"

void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset)
{
    interrupt_init();
    PIT_init(2000);
    disk_init(0, partitionOffset); // Mount our drive and partition
    // Welp, I guess we can write a FAT32 filesystem driver now, eh?

    kprintf("Hello from stage2!\n");
end:    
    for (;;);
}