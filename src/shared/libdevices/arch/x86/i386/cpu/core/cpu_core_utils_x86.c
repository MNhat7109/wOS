
#include <stdbool.h>
#include <devices/cpu/arch/x86/global/cio.h>
#include <devices/cpu/arch/x86/apic/apic_defs.h>

bool cpu_core_is_bsp(cio_layer_t* cpu_io)
{
    return (cpu_io->rdmsr(APIC_BASE_MSR) & APIC_BASE_BSP_FLAG) != 0;
}

bool cpu_core_x2apic_enabled(cio_layer_t* cpu_io)
{
    return (cpu_io->rdmsr(APIC_BASE_MSR) & APIC_BASE_X2APIC_ENABLE) != 0;
}

void cpu_core_enable_x2apic(cio_layer_t* cpu_io)
{
    u64 base = cpu_io->rdmsr(APIC_BASE_MSR);

    base |= APIC_BASE_X2APIC_ENABLE;

    cpu_io->wrmsr(APIC_BASE_MSR, base);
}

u32 cpu_core_get_bsp_runtime_lapic_id(cio_layer_t* cpu_io)
{
    return (u32)cpu_io->rdmsr(X2APIC_APICID_MSR);
}

void cpu_core_enable_xapic(cio_layer_t* cpu_io)
{
    u64 base = cpu_io->rdmsr(APIC_BASE_MSR);

    base |= APIC_BASE_APIC_GLOBAL_EN;

    cpu_io->wrmsr(APIC_BASE_MSR, base);
}