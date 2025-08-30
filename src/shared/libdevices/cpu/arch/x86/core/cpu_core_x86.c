#include <devices/cpu/cpu_core.h>
#include <devices/driver.h>

#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>
#include <devices/cpu/arch/x86/core/cpu_core_utils_x86.h>

#include <devices/cpu/arch/x86/apic/lapic.h>

#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>

#include <devices/cpu/cpu_core_utils.h>

static void cpu_core_config(struct generic_driver_tree_node_t* self);
static void cpu_core_probe(struct generic_driver_tree_node_t* self);
static void cpu_core_disable(struct generic_driver_tree_node_t* self);

struct cpu_core_driver_ops_t cpu_core_ops = {
    .ops_hdr = {
        .config = &cpu_core_config,
        .disable = &cpu_core_disable,
        .probe = &cpu_core_probe
    },
};

const struct generic_driver_ops_t* cpu_core_get_driver_ops()
{
    return &cpu_core_ops;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

static void cpu_core_config(struct generic_driver_tree_node_t* self)
{
    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "Needed params are not specified\n"
        );
        return;
    }

    // Check additional param
    x86_cpu_core_param_t* params = (x86_cpu_core_param_t*)self->additionals;

    // Check features
    if (!params->base->features->xapic)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "Bug: APIC mode not supported but continued anyway."
            " Halting\n"
        );
        driver_free(self, params);
        return;
    }

    // Compare with the suitable current (pre INIT+SIPI) 
    // LAPIC ID for the BSP status.

    params->is_bsp = cpu_core_is_bsp(params->base->cpu_io);
    
    // If the core is a BSP, enable every features (even x2APIC, mentioned above)
    
    if (params->is_bsp)
    {
        // Enable APIC mode
        cpu_core_enable_xapic(params->base->cpu_io);
        
        // Doing this so that it'll be easier to add new features and enable them.
        if (params->base->features->x2apic && !cpu_core_x2apic_enabled(params->base->cpu_io))
        {
            cpu_core_enable_x2apic(params->base->cpu_io);
            params->base->features->x2apic_on = 1;
            
            // Update the BSP's ID accordingly
            cpu_core_get_bsp_runtime_lapic_id(params->base->cpu_io);

            // Finally, init global LAPIC
            lapic_global_init(self->parent);
        }
    }

    // After that, do LAPIC initialization, and load necessary ops.
    lapic_init(self);
    GET_DEV_OPS(cpu_core, self)->core_ops = cpu_core_load_core_ops();
    GET_DEV_OPS(cpu_core, self)->timer_ops = cpu_core_load_timer_ops();
}

static void cpu_core_probe(struct generic_driver_tree_node_t* self)
{
    // No-op
}

static void cpu_core_disable(struct generic_driver_tree_node_t* self)
{
    // LAPIC disable
    lapic_disable(self);

    // Deallocate param block
    driver_free(self, self->additionals);
}
