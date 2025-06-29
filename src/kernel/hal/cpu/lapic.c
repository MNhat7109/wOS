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

void LAPIC_timer_init(u8 vector, u32 tick_count, u8 timer_mode, u8 divide_mode)
{
    // Example: div mode 3 -> 0b111
    // Low = 0b111&3 = 0b11
    // Hi = ((0b111>>2)&3)<<1 = 0b1 << 1 = 0b10
    // LAPIC div = low | (hi<<2) = 0b11 | (0b10<<2) = 0b1011
    u8 div_mode_lo = divide_mode & 0b11;
    u8 div_mode_hi = (divide_mode>>2) & 0b11;
    div_mode_hi <<= 1;
    LAPIC_write(LAPIC_REG_DIVCFG, div_mode_lo | (div_mode_hi<<2));
    LAPIC_write(LAPIC_REG_LVT, vector | (1<<16) | ((timer_mode&3)<<17));
    LAPIC_write(LAPIC_REG_INITCNT, tick_count);
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