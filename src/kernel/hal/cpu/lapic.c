#include "lapic.h"
#include "../../paging/paging.h"

volatile u32* lapic;

void LAPIC_write(u16 offset, u32 value)
{
    lapic[offset/4] = value;
}

u32 LAPIC_read(u16 offset)
{
    return lapic[offset/4];
}

bool LAPIC_init(u32 lapic_base)
{
    if (lapic_base == 0) return false;
    lapic = (u32*)lapic_base;

    return true;
}

void LAPIC_timer_init(u8 vector, u32 tick_count)
{
    LAPIC_write(0x3E0, 0b1011);
    LAPIC_write(0x320, vector | (1<<17));
    LAPIC_write(0x380, tick_count);
}

void LAPIC_send_eoi()
{
    LAPIC_write(0xB0, 0);
}

void LAPIC_detect(madt_record_entry_hdr_t* ptr)
{
    if (ptr->entry_type !=0) return;
    
}

u32 LAPIC_get_id()
{
    return (LAPIC_read(0x20) >> 24)&0xFF;
}

// void LAPIC_cpu_init()
// {

// }