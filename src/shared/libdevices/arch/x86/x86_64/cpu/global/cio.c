#include <devices/cpu/arch/x86/global/cio.h>

#include <libk/stdint.h>
#include <arch/x86/common/msr.h>

static void cio_cpuid(u32 leaf, u32 sub, u32* eax, u32* ebx, u32* ecx, u32* edx);
static void cio_wrmsr(u32 ecx, u64 msr);
static u64 cio_rdmsr(u32 ecx);

cio_layer_t cio_layer = {};

const cio_layer_t* cio_load_defaults()
{
    // Check for CPUID
    if (!_x86_has_cpuid())
    return NULL;

    cio_layer.cpuid = cio_cpuid;

    // Check for MSR
    u32 eax, ebx, ecx, edx;
    cio_cpuid(1, 0, &eax, &ebx, &ecx, &edx);

    if (!(edx & (1<<5)))
    return NULL;

    cio_layer.rdmsr = cio_rdmsr;
    cio_layer.wrmsr = cio_wrmsr;

    return &cio_layer;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

static void cio_cpuid(u32 leaf, u32 sub, u32* eax, u32* ebx, u32* ecx, u32* edx)
{
    _x86_cpuid(leaf, sub, eax, ebx, ecx, edx);
}

static void cio_wrmsr(u32 ecx, u64 msr)
{
    _x86_wrmsr(ecx, msr);
}

static u64 cio_rdmsr(u32 ecx)
{
    return _x86_rdmsr(ecx);
}