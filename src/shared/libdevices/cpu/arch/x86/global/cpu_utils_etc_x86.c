#include <libk/stdint.h>

#include <devices/cpu/arch/x86/global/cpu_utils_x86.h>
#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>
#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>
#include <devices/acpi/acpi.h>
#include <devices/driver.h>

static usize cpu_mps_type_to_len(usize type);

bool cpu_scan_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mps_drv_node,
    table_callback_t callback,
    void* ctx
)
{
    if (!mps_drv_node || !mps_drv_node->additionals)
        return false;

    mpct_hdr_t* mpct = (mpct_hdr_t*)mps_drv_node->additionals;

    u16 hdr_len = (mpct->spec_revision==4)?
    sizeof(mpct_hdr_t):
    sizeof(mpct_hdr_t)-sizeof(struct mpct_ext_t);

    void* record_start = (void*)((u8*)mpct+hdr_len);
    u8* entry = (u8*)record_start;

    usize total_len = mpct->length + ((mpct->spec_revision==4)?mpct->ext_fields.ext_length:0);

    usize rec_len = total_len-hdr_len;
    u8* entry_end = ((u8*)entry+rec_len);
    
    while (entry<entry_end)
    {
        u8* rec_entry = entry;
        usize len = cpu_mps_type_to_len(rec_entry[0]);
        if (len==0)
            break;
        
        callback(cpu_self, mps_drv_node,(void*)rec_entry, ctx);
        if (entry+len>entry_end) break;

        entry+=len;
    }

    return true;
}

void cpu_config_new_features(
    struct generic_driver_tree_node_t* self
)
{
    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Driver's additional data hasn't been allocated\n");
        return;
    }

    x86_cpu_global_param_t* param = (x86_cpu_global_param_t*)self->additionals;
    x86_cpu_global_additional_param_t* required = &param->export_params;
    x86_cpu_flags_t* feature_out = &param->global_cpu_flags;

    u32 eax, ebx, ecx, edx;
    required->cpu_io->cpuid(1, 0, &eax, &ebx, &ecx, &edx);

    feature_out->xapic = (edx >> 9)&1;
    feature_out->x2apic = (ecx>>21)&1;
    
    if (!feature_out->xapic)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "CPU does not support APIC mode. Cannot continue.\n"
        );
        return;
    }

    // Check max topology
    required->cpu_io->cpuid(0,0, &eax, &ebx, &ecx, &edx);

    if (eax >= 0x1F)
        feature_out->topology_0x1F = 1;
    else if (eax >= 0xB)
        feature_out->topology_0xB = 1;
    else
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "CPU does not support topology 0xB and up. Cannot continue.\n"
        );
        return;
    }
}

void cpu_config_table(
    struct generic_driver_tree_node_t* self,
    struct generic_driver_tree_node_t* acpi_drv_node
)
{
    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Driver's additional data hasn't been allocated\n");
        return;
    }

    x86_cpu_global_param_t* param = (x86_cpu_global_param_t*)self->additionals;
    x86_cpu_global_additional_param_t* required = &param->export_params;

    if (acpi_drv_node)
    {
        // Poke the ACPI driver to find the table for us
        required->madt_driver_node = 
        GET_DEV_OPS(acpi, acpi_drv_node)
        ->get_table(acpi_drv_node, "APIC");
        if (!required->madt_driver_node 
            || !required->madt_driver_node->additionals) // Weird, but ok...
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "MADT cannot be found. Subsequent per-core ops will use MSR and MPS.\n"
            );
        }

        // Register this as a resource node
        struct generic_driver_resource_t* madt_res_node =
        driver_request_resource(self, DRIVER_RES_TYPE_DEPENDENCY);
        if (self->state == DRIVER_STATE_FAILED)
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "MADT cannot be registered as a resource. Subsequent per-core ops will use MSR and MPS.\n"
            );
        }

        driver_set_res_data(self, madt_res_node, 
           required->madt_driver_node
        );
    }
    else
    {
        required->mps_driver_node = 
        cpu_create_mp_table_node(self);
        if (!required->mps_driver_node
            || !required->mps_driver_node->additionals)
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "MPS cannot be found. Multi-core support is nonexistent.\n"
            );
        }

        // Register this as a resource node
        struct generic_driver_resource_t* mps_res_node =
        driver_request_resource(self, DRIVER_RES_TYPE_DEPENDENCY);
        if (self->state == DRIVER_STATE_FAILED)
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "MPS cannot be registered as a resource. Multi-core support is nonexistent.\n"
            );
        }

        driver_set_res_data(self, mps_res_node, 
            required->mps_driver_node
        );
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

/*
* Translate the MPCT table type into its corresponding length.
* 
* As the tables' lengths are fixed (More detail on Intel MP Specification 1.4)
* (https://www.intel.com/content/www/us/en/io/mps-specification-archive.html),
* we can actually write a translation subroutine for this, e.g: Type 0 is 20 bytes,
* Type 1, 2, 3, 4 and 5 all have 8 bytes, etc.
*/
static usize cpu_mps_type_to_len(usize type)
{
    switch (type)
    {
        case 0:
            return 20;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return 8;
        default:
            return 0;
    }
}