#include <devices/acpi/acpi.h>
#include <devices/acpi/acpi_defs.h>
#include <devices/acpi/acpi_utils.h>

#include <libk/mmu/mmu.h>

typedef struct
{
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
} __attribute__((packed)) rsdp_t;

typedef struct
{
    rsdp_t rsdp;

    u32 length;
    u64 xsdt_addr;
    u8 ext_checksum;
    u8 reserved[3];
} __attribute__((packed)) xsdp_t;

static void acpi_config(struct generic_driver_tree_node_t* self);
static void acpi_disable(struct generic_driver_tree_node_t* self);
static void acpi_probe(struct generic_driver_tree_node_t* self);

struct acpi_driver_ops_t acpi_ops = {
    .ops_hdr = {
        .config = &acpi_config,
        .probe = &acpi_probe,
        .disable = &acpi_disable
    },
    .get_table = &acpi_find_table,
    .invalidate_table = &acpi_invalidate_table,
    .invalidate_all_tables = &acpi_invalidate_all_tables
};

const struct generic_driver_ops_t* acpi_get_driver_ops()
{
    return (struct generic_driver_ops_t*)&acpi_ops;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

typedef struct acpi_reserved_param_t 
{
    usize base, size, count, ptr_size;
} __attribute__((packed)) acpi_reserved_param_t;

static struct
{
    acpi_param_t saved_param_list;
} acpi_data;

static void acpi_config(struct generic_driver_tree_node_t* self)
{
    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_WARN, 
            "config() ran more than once. Skipping...\n"
        );
        return;
    }

    // Allocate the parameters to permanently use it as a container.

    self->additionals = driver_alloc(self, sizeof(acpi_param_t), DRIVER_ALLOC_FLAG_HEAP, 0);
    acpi_param_t* param = (acpi_param_t*)self->additionals;
    *param = acpi_data.saved_param_list;

    // Map the structure before doing anything, to be sure that
    // we do not touch unmapped memory
    mmu_mmap((usize)param->sys_desc_ptr, (usize)param->sys_desc_ptr, 0);
    
    system_desc_ptr_t* rxsdp = (system_desc_ptr_t*)param->sys_desc_ptr;
    acpi_sdt_hdr_t* rxsdt;
    u8 ptr_size;

    // Map the SDP's address, as it is our gateway to the RSDT/XSDT
    mmu_mmap(rxsdp->address, rxsdp->address, 0);

    if (rxsdp->type == SDP_TYPE_ACPI10)
    {
        rsdp_t* rsdp = (rsdp_t*)rxsdp->address;
        rxsdt = (acpi_sdt_hdr_t*)rsdp->rsdt_addr;
        ptr_size=4; // 4-byte pointers
    }
    else
    {
        xsdp_t* xsdp = (xsdp_t*)rxsdp->address;
        rxsdt = (acpi_sdt_hdr_t*)xsdp->xsdt_addr;
        ptr_size=8; // 8-byte pointers
    }
    
    // Map the RSDT header
    mmu_mmap((usize)rxsdt, (usize)rxsdt, 0);

    acpi_reserved_param_t* save_info
    = (acpi_reserved_param_t*)param->_reserved;
    
    // Save our needed info to all of our reserved slots
    save_info->base = (usize)((u8*)rxsdt+sizeof(acpi_sdt_hdr_t));
    save_info->ptr_size = ptr_size;

    save_info->size = rxsdt->length-sizeof(acpi_sdt_hdr_t);
    save_info->count = save_info->size/save_info->ptr_size;

    usize page_count = mmu_byte_to_page_count(save_info->size);
    mmu_mmapn(save_info->base, 0, page_count, 0); // Map as RO

    // All is done. Now to prevent unauthorized access or accessing the table's
    // value by mistake, we can now safely unmap the previously mapped RSDP and RSDT memory.
    mmu_munmap((usize)rxsdt);
    mmu_munmap(rxsdp->address);
    mmu_munmap((usize)rxsdp);
}

static void acpi_disable(struct generic_driver_tree_node_t* self)
{
    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, 
            "Parameter is not allocated. Check if config() was run before\n"
        );
        return;
    }

    acpi_param_t* param = (acpi_param_t*)self->additionals;
    acpi_reserved_param_t* save_info
    = (acpi_reserved_param_t*)param->_reserved;

    u64 page_count = mmu_byte_to_page_count(save_info->size);

    // First, we need to unmap the pointer list.
    mmu_munmapn(save_info->base, &page_count);

    // Then deallocate the parameters.
    driver_free(self, self->additionals);
}

static void acpi_probe(struct generic_driver_tree_node_t* self)
{
    // As the ACPI only requires one param to operate (the RSDP)
    // we can interpret the "additional" field in the node as such.

    // But, we actually want to utilize the container to store our saved
    // data to somewhere during the config() phase, so we will interpret
    // the mentioned field as a struct with one pointer (our input SDP),
    // and 3 reserved usize slots (the SDT list's base, its entry count
    // and the entry size)

    if (!self->additionals)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Parameter is empty\n");
        return;
    }
    
    acpi_data.saved_param_list = *(acpi_param_t*)self->additionals;
    
    void* sdp = acpi_data.saved_param_list.sys_desc_ptr;
    if (!sdp)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Cannot find bootloader RSDP/XSDP\n");
        return;
    }
}