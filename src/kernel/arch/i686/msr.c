#include <kernel/arch/i686/msr.h>
#include <kernel/arch/i686/cpuid.h>

#define MSR_SUPPORTED (1U<<5)

bool msr_check()
{
    if (!cpuid_check()) return false;

    cpuid_out_t cpuid_info;
    cpuid(CPUID_FUNC_GETFEATURES, &cpuid_info);
    
    return (cpuid_info.edx & MSR_SUPPORTED) != 0;
}