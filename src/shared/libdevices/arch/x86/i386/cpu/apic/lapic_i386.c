#include <devices/cpu/arch/x86/apic/lapic.h>
#include <devices/cpu/arch/x86/apic/apic_defs.h>

#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>
#include <devices/cpu/arch/x86/global/cio.h>
#include <devices/cpu/cpu_defs.h>
#include <devices/driver.h>

usize lapic_get_base(
    struct generic_driver_tree_node_t* cpu_self
)
{
    x86_cpu_global_param_t* param = (x86_cpu_global_param_t*)cpu_self->additionals;

    if (!param)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Parameter is empty\n");
        return 0;
    }

    // Obtain the LAPIC base
    // Preferably, we'll choose MADT first, then MPS, finally MSR fallback
    u32 lapic_base;

    if (param->export_params.madt_driver_node 
    && param->export_params.madt_driver_node->additionals)
    {
        madt_t* table = (madt_t*)param->export_params.madt_driver_node->additionals;
        lapic_base = table->lapic_addr;
    }
    else if (param->export_params.mps_driver_node
    && param->export_params.mps_driver_node->additionals)
    {
        mpct_hdr_t* table = (mpct_hdr_t*)param->export_params.mps_driver_node->additionals;
        lapic_base = table->lapic_addr;
    }
    else
    lapic_base = param->export_params.cpu_io->rdmsr(APIC_BASE_MSR);

    return lapic_base;
}