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
    return &acpi_ops;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

static struct
{
    acpi_param_t* param_list;
    system_desc_ptr_t* rxsdp;
    acpi_sdt_hdr_t* rxsdt;
    void* sdtp_list_base;
    usize sdtp_list_page_cnt;
} acpi_data;

static void acpi_config(struct generic_driver_tree_node_t* self)
{
    // Map the structure before doing anything, to be sure that
    // we do not touch unmapped memory
    mmu_mmap((usize)acpi_data.rxsdp, (usize)acpi_data.rxsdp, 0);
    u8 ptr_size;

    // Map the SDP's address, as it is our gateway to the RSDT/XSDT
    mmu_mmap(acpi_data.rxsdp->address, acpi_data.rxsdp->address, 0);

    if (acpi_data.rxsdp->type == SDP_TYPE_ACPI10)
    {
        rsdp_t* rsdp = (rsdp_t*)acpi_data.rxsdp->address;
        acpi_data.rxsdt = (acpi_sdt_hdr_t*)rsdp->rsdt_addr;
        ptr_size=4; // 4-byte pointers
    }
    else
    {
        xsdp_t* xsdp = (xsdp_t*)acpi_data.rxsdp->address;
        acpi_data.rxsdt = (acpi_sdt_hdr_t*)xsdp->xsdt_addr;
        ptr_size=8; // 8-byte pointers
    }
    
    // Map the RSDT header
    mmu_mmap((usize)acpi_data.rxsdt, (usize)acpi_data.rxsdt, 0);

    // === Local scratchpad copy ===
    // NOTE: _reserved is just temporary storage. We copy its content
    // into a local save_info struct, modify it, and then throw it away.
    // No other subsystem should rely on values "persisting" in _reserved.

    struct save_t {usize base, count, ptr_size;} __attribute__((packed)) save_info
    = *(struct save_t*)acpi_data.param_list->_reserved;
    
    // Save our needed info to all of our reserved slots
    save_info.base = (u8*)acpi_data.rxsdt+sizeof(acpi_sdt_hdr_t);
    acpi_data.sdtp_list_base = save_info.base;

    save_info.ptr_size = ptr_size;

    usize sdts_size = acpi_data.rxsdt->length-sizeof(acpi_sdt_hdr_t);
    save_info.count = sdts_size/save_info.ptr_size;

    usize page_count = mmu_byte_to_page_count(sdts_size);
    mmu_mmapn(save_info.base, 0, page_count, 0); // Map as RO
    acpi_data.sdtp_list_page_cnt = page_count;

    // All is done. Now to prevent unauthorized access or accessing the table's
    // value by mistake, we can now safely unmap the previously mapped RSDP and RSDT memory.
    mmu_munmap((usize)acpi_data.rxsdt);
    mmu_munmap(acpi_data.rxsdp->address);
    mmu_munmap((usize)acpi_data.rxsdp);
}

static void acpi_disable(struct generic_driver_tree_node_t* self)
{
    // Now on disable, we only need to unmap the pointer list.
    mmu_munmapn(acpi_data.sdtp_list_base, acpi_data.sdtp_list_page_cnt);
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

    acpi_data.param_list = (acpi_param_t*)self->additionals;
    void* sdp = acpi_data.param_list->sys_desc_ptr;
    if (!sdp)
    {
        driver_log_state(self, DRIVER_LOG_ERROR, "Cannot find bootloader RSDP/XSDP\n");
        return;
    }

    acpi_data.rxsdp = (system_desc_ptr_t*)sdp;
}