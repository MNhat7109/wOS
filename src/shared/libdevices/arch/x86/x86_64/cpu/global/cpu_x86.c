#include <devices/driver.h>
#include <devices/cpu/cpu.h>
#include <devices/cpu/cpu_core.h>
#include <devices/cpu/cpu_utils.h>

#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>
#include <devices/cpu/arch/x86/global/cpu_utils_x86.h>
#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>

#include <devices/cpu/arch/x86/apic/ioapic.h>
#include <devices/acpi/acpi.h>
#include <devices/cpu/arch/x86/global/cio.h>

static void cpu_config(struct generic_driver_tree_node_t* self);
static void cpu_disable(struct generic_driver_tree_node_t* self);
static void cpu_probe(struct generic_driver_tree_node_t* self);

struct cpu_driver_ops_t cpu_ops = {
    .ops_hdr = {
        .config = &cpu_config,
        .disable = &cpu_disable,
        .probe = &cpu_probe
    },
};

const struct generic_driver_ops_t* cpu_get_driver_ops()
{
    return (struct generic_driver_ops_t*)&cpu_ops;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

static void cpu_config(struct generic_driver_tree_node_t* self)
{    
    x86_cpu_global_param_t* x86_cpu_data = (x86_cpu_global_param_t*)self->additionals;
    if (!x86_cpu_data)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Check if probe() has been run\n");
        return;
    }

    // Check for MSR support and CPUID support
    // All of the said checks will be in the 
    // cio_load_defaults().

    x86_cpu_data->export_params.cpu_io = cio_load_defaults();
    if (!x86_cpu_data->export_params.cpu_io)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "CIO init failed."
            " Check if the CPU supports CPUID and MSR.\n"
        );
        return;
    }

    // Probe for new features
    cpu_config_new_features(self);
    if (self->state == DRIVER_STATE_FAILED) return;
    x86_cpu_data->export_params.features = &x86_cpu_data->global_cpu_flags;

    // Prepare tables for future core discovery and IOAPIC
    cpu_config_table(self);
    if (self->state == DRIVER_STATE_FAILED) return;
    
    // Start up CPU cores
    driver_log_state(self, DRIVER_LOG_NOTICE, 
        "Detecting CPU cores...\n"
    );
    
    cpu_enumerate_cores(self);
    if (self->state == DRIVER_STATE_FAILED)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "Cannot enumerate CPU cores. Further operation cannot be continued.\n"
        );
        return;
    }
    
    // Load the global IOAPIC
    
    driver_log_state(self, DRIVER_LOG_NOTICE, 
        "Loading global interrupt ops...\n"
    );

    ioapic_init(self, &x86_cpu_data->export_params);
    if (self->state == DRIVER_STATE_FAILED)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Failed to load the ops.\n");
        return;
    }
    GET_DEV_OPS(cpu, self)->global_interrupt_ops = cpu_load_int_ops();
}

static void cpu_disable(struct generic_driver_tree_node_t* self)
{
    x86_cpu_global_param_t* x86_cpu_data = (x86_cpu_global_param_t*)self->additionals;

    if (!x86_cpu_data)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Check if probe() has been run\n");
        return;
    }

    // Disable child core nodes
    for (struct generic_driver_tree_node_t* child = self->first_child;
    child;)
    {
        struct generic_driver_tree_node_t* next = child->next_peer;
        if(!driver_terminate(child))
            driver_log_state(self, DRIVER_LOG_WARN, "Failed to terminate core node\n");
        else
            driver_remove_from_tree(&self, child);
        child=next;
    }

    // Disable IOAPIC
    ioapic_disable(self);
    if (self->state == DRIVER_STATE_FAILED)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "Failed to disable the global interrupt ops.\n");
        return;
    }

    // Untie resource
    for (struct generic_driver_resource_t* res = self->resource_list;
    res; res=res->next)
    {
        bool status = driver_untie_resource(self, res);
        if (!status)
        {
            driver_log_state(self, DRIVER_LOG_WARN, "Failed to remove resource node\n");
            continue;
        }
    }

    // Remove dependencies
    if (x86_cpu_data->export_params.madt_driver_node)
    GET_DEV_OPS(acpi, x86_cpu_data->acpi_driver_node)->invalidate_all_tables(self,
    x86_cpu_data->export_params.madt_driver_node);
    
    // In case the MPS is used, and therefore is created, remove that too.
    if (x86_cpu_data->export_params.mps_driver_node)
        cpu_remove_mp_table_node(self, x86_cpu_data->export_params.mps_driver_node);

    // Free params
    driver_free(self, self->additionals);
}

static void cpu_probe(struct generic_driver_tree_node_t* self)
{
    // Allocate memory for parameter
    self->additionals = driver_alloc(
        self, 
        sizeof(x86_cpu_global_param_t), 
        DRIVER_ALLOC_FLAG_HEAP, 0
    );

    x86_cpu_global_param_t* x86_cpu_data = (x86_cpu_global_param_t*)self->additionals;

    // NOTE: This is for x86 only.

    // As our CPU driver will be highly dependent on the MADT table (ACPI)
    // or the MPT table (non-ACPI), we will do this in two steps:
    // Request ACPI driver first, if not found or failed, we use MPT as a fallback.
    
    // Request ACPI driver
    // As the CPU driver is of the "Root device" category, we can search in its peer list to save time

    struct generic_driver_tree_node_t* acpi_driver_node =
    driver_get_by_id(&self, "GENERIC_ACPI_ROOT_DEV");

    if (acpi_driver_node && driver_run(acpi_driver_node))
    {
        // ACPI driver node is alive and well in the forest,
        // Set up resource for that.
        struct generic_driver_resource_t* res_node =
        driver_request_resource(self, DRIVER_RES_TYPE_DEPENDENCY);
        if (self->state == DRIVER_STATE_FAILED) return;

        driver_set_res_data(self, res_node, acpi_driver_node);

        driver_log_state(self, DRIVER_LOG_NOTICE, 
            "ACPI device node found. Will scan MADT for APICs\n"
        );
        x86_cpu_data->acpi_driver_node = acpi_driver_node;
        return;
    }

    // Fallback: Use MPT, but notify the client first.
    driver_log_state(self, DRIVER_LOG_WARN, "ACPI device node not found or failed."
        " Will scan MPT for APICs instead.\n"
    );
}
