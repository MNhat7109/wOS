#include <kernel/arch/i686/cpuid.h>
#include <kernel/debug.h>

#define MODULE_CPU "CPU"

void __attribute__((cdecl)) cpuid_notify_unsupported()
{
    kdebugf(DEBUG_WARN, MODULE_CPU, "CPUID is not supported in this machine.\n");
}

void __attribute__((cdecl)) cpuid_notify_error()
{
    kdebugf(DEBUG_CRITICAL, MODULE_CPU, "Cannot use CPUID. Check if CPUID is supported in this machine.\n");
}

void __attribute__((cdecl)) cpuid(u32 func, u32 cpu_info_out[4])
{
    cpuidex(func, 0, cpu_info_out);
}