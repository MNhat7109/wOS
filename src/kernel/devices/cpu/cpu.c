#include "apic/apic_defs.h"
#include "apic/lapic.h"
#include "apic/ioapic.h"

#include "cpu.h"
#include "cpu_defs.h"
#include "cpu_utils.h"
#include "../acpi/acpi.h"
#include "../../stdio.h"
#include "../../paging/paging.h"

struct x86_cpu_shared_t x86_generic_cpu;

void cpu_config(struct generic_driver_t* driver)
{
    // Let the layers sip the driver up
    x86_generic_cpu.lapic_layer->cpu_dev = (struct cpu_driver_t*)driver;
    x86_generic_cpu.ioapic_layer->cpu_dev = (struct cpu_driver_t*)driver;

    if (!driver_run((struct generic_driver_t*)x86_generic_cpu.lapic_layer))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR,
             "LAPIC failed to start. Exiting...");
        return;
    }

    if (!driver_run((struct generic_driver_t*)x86_generic_cpu.ioapic_layer))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR,
             "IOAPIC failed to start. Exiting...");
        return;
    }
}

void cpu_probe(struct generic_driver_t* driver)
{
    struct cpu_driver_t* cpu_self = (struct cpu_driver_t*)driver;

    // Check MSR and CPUID support
    cpu_feature_check(&x86_generic_cpu.msr_support, &x86_generic_cpu.cpuid_support);
    if (x86_generic_cpu.msr_support) cpu_self->cpu_io_layer = cio_load_defaults();

    // Run ACPI driver
    x86_generic_cpu.acpi_driver = (struct acpi_driver_t*)driver_get("ACPI");

    
    if (!driver_run((struct generic_driver_t*)x86_generic_cpu.acpi_driver))
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE,
             "ACPI driver failed to start. Will use MSR to find APIC instead");
        x86_generic_cpu.acpi_support = false;
    }
    else
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE,
             "ACPI MADT will be used to init APIC");
        x86_generic_cpu.acpi_support = true;
    } 

    // Find MADT
    if (!cpu_prepare_apic_acpi())
    {
        if (!cpu_prepare_apic_msr(cpu_self))
        {
            driver_log_state(driver, DRIVER_LOG_ERROR, "Failed to prepare for APIC");
            return;
        }
    }
}

void cpu_disable(struct generic_driver_t* driver)
{
    if (x86_generic_cpu.acpi_support&&
        !page_manager_unmap_memory((u32)x86_generic_cpu.madt))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR,
             "Cannot unmap MADT table");
        return;
    }

    if (!driver_terminate((struct generic_driver_t*)x86_generic_cpu.lapic_layer))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR,
             "LAPIC failed to terminate. Exiting...");
        return;
    }

    if (!driver_terminate((struct generic_driver_t*)x86_generic_cpu.ioapic_layer))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR,
             "IOAPIC failed to terminate. Exiting...");
        return;
    }
}

struct cpu_driver_t cpu_driver = {
    .driver_hdr = {
        .name = "CPU",
        .config = &cpu_config,
        .probe = &cpu_probe,
        .disable = &cpu_disable,
        .state = DRIVER_STATE_UNPROBED
    },
    .scan_from_madt = &cpu_scan_madt,
    .scan_from_mpt = &cpu_scan_mpt,
    .send_eoi = &cpu_send_eoi,
    .get_core_id = &cpu_get_core_id,
    .normalize_irq = &cpu_irq_to_gsi,
    .redirect_gsi = &cpu_redirect_gsi,
    .cut_gsi = &cpu_cut_gsi,
    .timer_countdown = &cpu_timer_countdown,
    .timer_get_freq = &cpu_timer_get_freq,
    .timer_init = &cpu_timer_init
};

const struct generic_driver_t* cpu_get_driver()
{
    x86_generic_cpu.lapic_layer = lapic_get_driver();
    x86_generic_cpu.ioapic_layer = ioapic_get_driver();
    return (struct generic_driver_t*)&cpu_driver;
}
