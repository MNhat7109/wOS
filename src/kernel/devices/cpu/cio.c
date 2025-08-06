#include "cio.h"
#include "../../x86/x86.h"

void cpu_feature_check(u8* msr, u8* cpuid)
{
    _x86_cpu_check(msr, cpuid);
}

void cio_wrmsr(u32 ecx, u64 value)
{
    _x86_wrmsr(ecx, value);
}

u64 cio_rdmsr(u32 ecx)
{
    return _x86_rdmsr(ecx);
}

cpu_io_t cio = {};
const cpu_io_t cio_load_defaults()
{
    return cio;
}

void cio_load_msr()
{
    cio.rdmsr = &cio_rdmsr;
    cio.wrmsr = &cio_wrmsr;
}