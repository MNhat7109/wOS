#include <devices/cpu/cpu_core_utils.h>
#include <devices/cpu/cpu_core.h>
#include <devices/cpu/cpu_defs.h>

#include <devices/driver.h>
#include <libk/stdio.h>

struct generic_driver_tree_node_t* cpu_create_core_node(
    struct generic_driver_tree_node_t* cpu_self,
    u32 index
)
{
    struct generic_driver_tree_node_t* new_node = driver_add_to_tree(
        &driver_forest,
        cpu_self, DRIVER_ID_TYPE_INTERNAL,
        DRIVER_BUS_TYPE_CPU,
        100,
        DRIVER_MODE_KRNL
    );
    if (!new_node)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Cannot init BSP core node\n");
        return;
    }

    // Cook up CID
    char cid[32];
    ksnprintf(cid, sizeof(cid), "GENERIC_CPU_CORE%u", index);
    driver_set_id_data(new_node, cid);
    
    // Load default ops
    driver_load_ops(new_node, cpu_core_get_driver_ops);

    return new_node;
}

bool cpu_scan_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_drv_node,
    table_callback_t callback,
    void* ctx
)
{
    if (!madt_drv_node || !madt_drv_node->additionals)
        return false;

    madt_t* madt = (madt_t*)madt_drv_node->additionals;

    madt_record_entry_hdr_t* record_start = (madt_record_entry_hdr_t*)((u8*)madt+sizeof(madt_t));
    u8* entry = (u8*)record_start;
    
    usize rec_len = madt->table_hdr.length-sizeof(madt_t);
    u8* entry_end = ((u8*)entry+rec_len);
    
    while (entry<entry_end)
    {
        madt_record_entry_hdr_t* rec_entry = (madt_record_entry_hdr_t*)entry;
        u8 entry_type = rec_entry->entry_type;
        u8 len = rec_entry->record_length;

        if (len < 2 || entry+len>entry_end) break;
        callback(cpu_self, madt_drv_node, (void*)rec_entry, ctx);
        entry+=len;
    }

    return true;
}