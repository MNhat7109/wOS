#include "stdint.h"
#include "stdio.h"
#include "interrupt/interrupt.h"


void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset)
{
    interrupt_init();
    kprintf("Hello from stage2!\n");
end:    
    for (;;);
}