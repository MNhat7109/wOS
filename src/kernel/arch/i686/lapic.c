#include <kernel/arch/i686/apic.h>
#include <kernel/arch/i686/msr.h>
#include <kernel/mmu.h>
#include <kernel/mmio.h>

static struct
{
    u32 lapic_base;
} lapic_data;

int lapic_init(apic_tbl_t* madt_cont)
{
    if (madt_cont)
    {
        lapic_data.lapic_base = madt_cont->madt->lapic_addr;
    }
    else
    {
        if (!msr_check()) return -1;

        lapic_data.lapic_base = msr_read(0x1B);
        mmu_align_down(lapic_data.lapic_base, PAGE_SIZE);
    } 

    int st = mmio_setup_addr((void*)lapic_data.lapic_base, PAGE_SIZE);
    if (st < 0) return st;
    return 0;
}

void lapic_ack()
{

}