#include <devices/acpi/acpi_utils.h>
#include <devices/acpi/acpi_defs.h>
#include <devices/driver.h>

#include <libk/string.h>
#include <libk/stdio.h>
#include <libk/utils/verify.h>

typedef struct
{
    usize base, entry_count, ptr_size;
} __attribute__((packed)) acpi_sdt_list_info_t;

bool acpi_table_is_sane(acpi_sdt_hdr_t* hdr)
{
    // If the header is NULL, reject.
    if (!hdr) return false;

    // If the table length is less than the header size, reject.
    if (hdr->length < sizeof(acpi_sdt_hdr_t)) return false;

    // If the table is too large (more than 1 MiB), reject.
    if (hdr->length > (1<<20)) return false;

    // Check for alignment.
    if (((usize)hdr & 0xF) != 0) return false;

    return true;
}

struct generic_driver_tree_node_t* acpi_find_table(
    struct generic_driver_tree_node_t* self, 
    char* signature
)
{
    if (!self->additionals) return NULL;
    acpi_param_t* param_list = (acpi_param_t*)self->additionals;
    acpi_sdt_list_info_t* list_info = 
    (acpi_sdt_list_info_t*)(param_list->_reserved);
    struct generic_driver_tree_node_t* table_drv_node = NULL;

    for (usize i=0;i<list_info->entry_count;i++)
    {
        u64 table_base;
        memcpy(
            &table_base, 
            list_info->base+i*list_info->ptr_size, 
            list_info->ptr_size
        );

        // We have previously mapped the whole list, so no need to do it again
        acpi_sdt_hdr_t* hdr = (acpi_sdt_hdr_t*)table_base;

        if (memcmp(hdr->signature, signature, 4) != 0) continue;

        driver_log_state(self, DRIVER_LOG_NOTICE, 
            "For header with signature: %s, base=0x%p:\n",
            signature, hdr
        );

        // Checks

        if (!acpi_table_is_sane(hdr))
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "Sanity check failed, skipping...\n"
            );
            continue;
        }

        if (!verify_8bit_checksum(hdr, hdr->length))
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "Checksum invalid, skipping...\n"
            );
            continue;
        }

        driver_log_state(self, DRIVER_LOG_NOTICE, "Header found.\n");

        // Cook up cid
        char cid[22]; // 17 prefix + 4 sig + 1 terminator
        ksnprintf(cid, sizeof(cid), "GENERIC_ACPI_TBL_%s", signature);

        // Find child node with the cid
        table_drv_node = driver_get_by_id(
            &driver_forest,
            cid
        );

        if (!table_drv_node)
        {
            // If the driver's node does not exist,
            // create one

            table_drv_node = driver_add_to_tree(
                &driver_forest,
                self,
                DRIVER_ID_TYPE_INTERNAL,
                DRIVER_BUS_TYPE_ACPI,
                50,
                DRIVER_MODE_KRNL
            );
            if (!table_drv_node)
            {
                driver_log_state(self, DRIVER_LOG_WARN, 
                    "Out of memory trying to create a new list node\n"
                );
                return NULL;
            }
    
            driver_set_id_data(table_drv_node, cid);
            table_drv_node->state = DRIVER_STATE_READY; // Set it to Ready, as a no-op driver
        }

        // Interpret the additional param as a head, and add new node
        // accordingly
        acpi_sdt_list_t* new_table_list_node = acpi_table_add_to_list(
            self, (acpi_sdt_list_t**)&table_drv_node->additionals,
            hdr
        );

        if (!new_table_list_node) 
        {
            driver_log_state(self, DRIVER_LOG_WARN, 
                "Out of memory trying to create a new list node\n"
            );
            break;
        }
    }

    if (!table_drv_node || !table_drv_node->additionals)
    {
        driver_log_state(self, DRIVER_LOG_WARN, 
            "Cannot find any valid headers with signature: %s\n",
            signature
        );
        return NULL;
    }
    return table_drv_node;
}

bool acpi_invalidate_table(
    struct generic_driver_tree_node_t* self, 
    struct generic_driver_tree_node_t* table_drv_node,
    acpi_sdt_list_t* table_list_node
)
{
    // If the list node or the driver is NULL, bail out.
    if (!table_drv_node || !table_list_node)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "Unspecified table or node\n");
        return false;
    }

    char disp_signature[5];
    memcpy(disp_signature, table_list_node->hdr->signature, 4);
    disp_signature[4] = '\0';
    
    driver_log_state(self, DRIVER_LOG_NOTICE,
    "ACPI table offset=0x%p, signature: %s is being invalidated...\n",
    table_list_node->hdr, disp_signature);

    // Unlink the table
    acpi_sdt_list_t** link = (acpi_sdt_list_t**)&table_drv_node->additionals;

    while (*link && *link != table_drv_node)
        link = &(*link)->next;
    
    if (!*link) 
    {
        driver_log_state(self, DRIVER_LOG_WARN, "Cannot find the node specified\n");
        return false;
    }

    *link = table_list_node->next;
    table_list_node->next = NULL;

    // Finally, free the list node.
    driver_free(self, table_list_node);

    // If the list is empty (NULL), we can safely remove the
    // driver node.
    if (!table_drv_node->additionals)
    {
        driver_log_state(self, DRIVER_LOG_NOTICE,
            "ACPI table list is empty. The table's driver node will be removed.\n");
        
        driver_remove_from_tree(
            &driver_forest,
            table_drv_node
        );
    }

    return true;
}

bool acpi_invalidate_all_tables(
    struct generic_driver_tree_node_t* self, 
    struct generic_driver_tree_node_t* table_drv_node
)
{
    // If the driver node is NULL, bail out.
    if (!table_drv_node) 
    {
        driver_log_state(self, DRIVER_LOG_WARN, "Unspecified table\n");
        return false;
    }

    // If the table list is NULL, bail out.
    if (!table_drv_node->additionals) 
    {
        driver_log_state(self, DRIVER_LOG_WARN, "Table list is empty\n");
        return false;
    }

    acpi_sdt_list_t* list = (acpi_sdt_list_t*)table_drv_node->additionals;

    while (list)
    {
        acpi_sdt_list_t* next = list->next;

        // Call existing per-table invalidator
        bool status = acpi_invalidate_table(
            self,
            table_drv_node,
            list
        );

        // If table_node got deleted inside invalidate_table, bail
        if (!table_drv_node || !table_drv_node->additionals)
            break;

        list = next;
    }
    
    driver_log_state(self, DRIVER_LOG_NOTICE,
        "ACPI tables are now fully invalidated\n");
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static acpi_sdt_list_t* acpi_table_add_to_list(
    struct generic_driver_tree_node_t* self,
    acpi_sdt_list_t** head,
    acpi_sdt_hdr_t* hdr
)
{
    if (!hdr) return NULL;

    acpi_sdt_list_t* new_node = driver_alloc(
        self, sizeof(acpi_sdt_list_t*),
        DRIVER_ALLOC_FLAG_HEAP,
        0 // If the heap is used, then MMU flags are ignored
    );
    if (!new_node) return NULL;

    new_node->hdr = hdr;
    new_node->next = NULL;

    if (!*head)
    {
        *head = new_node;
        return new_node;
    }

    acpi_sdt_list_t* current_node = *head;
    while (current_node->next) current_node = current_node->next;
    current_node->next = new_node;
    return new_node;
}