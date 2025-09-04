#include <devices/driver.h>

#include <devices/cpu/cpu_utils.h>
#include <devices/cpu/arch/x86/global/cpu_utils_x86.h>
#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>
#include <devices/cpu/arch/x86/global/cio.h>

#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>
#include <devices/cpu/cpu_core.h>

#define MAX_NUM_CPU 8192

#define ACPI_PROC_ENABLE (1<<0)
#define ACPI_PROC_ONLINE_CAPABLE (1<<1)

#define MPCT_PROC_ENABLE (1<<0)

typedef struct
{
    madt_record_entry_hdr_t record_hdr;
    u8 acpi_proc_id;
    u8 lapic_id;
    u32 flags;
} __attribute__((packed)) madt_record_proc_lapic_t;

typedef struct
{
    madt_record_entry_hdr_t record_hdr;
    u16 _reserved;
    u32 lx2apic_id;
    u32 flags;
    u32 acpi_proc_id;
} __attribute__((packed)) madt_record_proc_lx2apic_t;

typedef struct
{
    u8 type;
    u8 lapic_id;
    u8 lapic_version;
    u8 cpu_flags;
    u32 cpu_signature;
    u32 feature_flags;
    u64 _reserved;
} __attribute__((packed)) mpct_proc_entry_t;

int current_core_count = 0;

static void cpu_core_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
);
static void cpu_core_handle_x2apic(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
);
static void cpu_core_detect_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
);

/*
* Detect and enumerate over CPU cores
* For multi-core support
*
* NOTE: The BSP core node (and other similar AP core nodes)
* will take two params: The MADT node and the MPCT node.
*
* The core nodes can just check if either of these are found (not NULL), then it'll set up
* their additional ops (timer, core, int) with the found table.
*
* Pretty neat, eh?
*/
void cpu_enumerate_cores(
    struct generic_driver_tree_node_t* cpu_self
)
{
    // If the required is NULL, return
    if (!cpu_self->additionals)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, 
            "Needed param for per-core nodes is not passed.\n");
        return;
    }

    x86_cpu_global_param_t* param = (x86_cpu_global_param_t*)cpu_self->additionals;
    x86_cpu_global_additional_param_t* required = &param->export_params;

    // We always prefer ACPI's MADT over MPCT

    if (required->madt_driver_node && required->madt_driver_node->additionals)
    {
        // Scan the MADT for type 0 records
        driver_log_state(cpu_self, DRIVER_LOG_NOTICE, 
            "MADT found. Getting core info in MADT...\n"
        );

        cpu_scan_madt(
            cpu_self, 
            required->madt_driver_node, 
            cpu_core_detect_with_madt, required
        );
        if (current_core_count == 0)
        {
            driver_log_state(cpu_self, DRIVER_LOG_WARN, 
                "Cannot find any core info in the MADT. Will assume single core from now on.\n"
            );
        }

        if (required->features->x2apic)
        cpu_scan_madt(
            cpu_self, 
            required->madt_driver_node, 
            cpu_core_handle_x2apic, required
        );
    }
    else if (required->mps_driver_node && required->mps_driver_node->additionals)
    {
        // Scan the MPCT for type 0 records
        driver_log_state(cpu_self, DRIVER_LOG_NOTICE, 
            "MPS found. Getting core info in MPS...\n"
        );

        cpu_scan_mps(cpu_self, required->mps_driver_node, cpu_core_detect_with_mps, required);
        if (current_core_count == 0)
        {
            driver_log_state(cpu_self, DRIVER_LOG_WARN, 
                "Cannot find any core info in the MPS. Will assume single core from now on.\n"
            );
        }
    }
    else
    {
        driver_log_state(cpu_self, DRIVER_LOG_NOTICE, 
            "Initializing BSP as sole core node...\n"
        );

        struct generic_driver_tree_node_t* bsp_core_node = 
        cpu_create_core_node(cpu_self, 0);

        x86_cpu_core_param_t* param = driver_alloc(
            bsp_core_node, 
            sizeof(x86_cpu_core_param_t),
            DRIVER_ALLOC_FLAG_HEAP,
            0
        );
        if (!param)
        {
            driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Failed to allocate param for BSP\n");
            driver_remove_from_tree(NULL, bsp_core_node);
            return;
        }

        param->base = required;
        param->lapic_id = required->cpu_io->rdmsr(0x1B);
        param->proc_id = 0;

        bsp_core_node->additionals = param;

        
        driver_load_ops(bsp_core_node, cpu_core_get_driver_ops); // Important, else it won't work
        if (!driver_run(bsp_core_node))
        {
            driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Failed to run BSP core ops\n");
            driver_free(bsp_core_node, param);
            driver_remove_from_tree(NULL, bsp_core_node);
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void cpu_core_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
)
{
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)record;
    if (p->entry_type != 0) return;

    if (current_core_count >= MAX_NUM_CPU) 
        return;

    madt_record_proc_lapic_t* proc_ptr = (madt_record_proc_lapic_t*)record;

    // Check if the CPU core is online capable
    if (!(proc_ptr->flags & ACPI_PROC_ONLINE_CAPABLE))
        return;
    
    struct generic_driver_tree_node_t* core_node = 
        cpu_create_core_node(cpu_self, current_core_count);

    x86_cpu_core_param_t* param = driver_alloc(
        core_node, 
        sizeof(x86_cpu_core_param_t),
        DRIVER_ALLOC_FLAG_HEAP,
        0
    );
    if (!param)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Failed to allocate param for BSP\n");
        driver_remove_from_tree(NULL, core_node);
        return;
    }

    param->base = (x86_cpu_global_additional_param_t*)ctx;
    param->proc_id = current_core_count;
    param->acpi_proc_id = proc_ptr->acpi_proc_id;
    param->lapic_id = proc_ptr->lapic_id;
    param->flags = proc_ptr->flags;

    core_node->additionals = param;

    driver_load_ops(core_node, cpu_core_get_driver_ops); // Important, else it won't work
    if (!driver_run(core_node))
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Failed to run BSP core ops\n");
        driver_free(core_node, param);
        driver_remove_from_tree(NULL, core_node);
        return;
    }
    current_core_count++;
}

static void cpu_core_handle_x2apic(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
)
{
    (void)ctx;
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)record;
    if (p->entry_type != 9) return;

    madt_record_proc_lx2apic_t* lx2apic_ptr = (madt_record_proc_lx2apic_t*)p;

    // Find the suitable ACPI ID
    for (struct generic_driver_tree_node_t* child = cpu_self->first_child;
    child; child = child->next_peer)
    {
        x86_cpu_core_param_t* params = (x86_cpu_core_param_t*)child->additionals;
        if (!params) 
        {
            driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Param is empty\n");
            return;
        }

        if (params->acpi_proc_id == lx2apic_ptr->acpi_proc_id)
        {
            params->lapic_id = lx2apic_ptr->lx2apic_id;
            params->flags = lx2apic_ptr->flags;
            return;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static void cpu_core_detect_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
)
{
    mpct_proc_entry_t* proc_ptr = (mpct_proc_entry_t*)record;
    
    if (proc_ptr->type != 0) return;

    if (current_core_count >= MAX_NUM_CPU) return;

    // Check if the CPU is even capable to going online
    if (!(proc_ptr->cpu_flags & MPCT_PROC_ENABLE)) return;

    struct generic_driver_tree_node_t* core_node = 
        cpu_create_core_node(cpu_self, current_core_count);

    x86_cpu_core_param_t* param = driver_alloc(
        core_node, 
        sizeof(x86_cpu_core_param_t),
        DRIVER_ALLOC_FLAG_HEAP,
        0
    );
    if (!param)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Failed to allocate param for BSP\n");
        driver_remove_from_tree(NULL, core_node);
        return;
    }

    param->base = (x86_cpu_global_additional_param_t*)ctx;
    param->proc_id = current_core_count;
    param->acpi_proc_id = 0; // No ACPI, why bother?
    param->lapic_id = proc_ptr->lapic_id;

    core_node->additionals = param;
}
