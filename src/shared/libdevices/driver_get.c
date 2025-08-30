#include <devices/driver.h>

#include <libk/string.h>
#include <libk/stdatomic.h>
#include <stdarg.h>

#include <libk/containers/queue.h>

#define MAX_DRIVERS_FOUND 128
struct generic_driver_tree_node_t* found_list[MAX_DRIVERS_FOUND];

static struct generic_driver_tree_node_t* driver_choose_best_node(
    struct generic_driver_tree_node_t* found_list[],
    usize items_found
);

usize driver_get_all_customized(
    struct generic_driver_tree_node_t** root, 
    struct generic_driver_tree_node_t** found_list,
    bool (*id_method)(struct generic_driver_tree_node_t*, void*),
    void* id_data
)
{
    usize found = 0;
    usize size = sizeof(found_list)/sizeof(struct generic_driver_tree_node_t*);

    // BFS!
    QUEUE(struct generic_driver_tree_node_t*) queue;
    QUEUE_INIT(queue);
    if (!queue.ptr) return 0;

    QUEUE_PUSH(queue, struct generic_driver_tree_node_t*, *root);

    while (!QUEUE_EMPTY(queue))
    {
        struct generic_driver_tree_node_t* current_node = 
        QUEUE_FRONT(queue, struct generic_driver_tree_node_t*);
        QUEUE_POP(queue);

        if (!id_method) break;
        if (id_method(current_node, id_data))
        {
            if (found+1 >= size) break;
            found_list[found++] = current_node;
        }

        // Push one peer, one child
        if (current_node->next_peer)
        {
            QUEUE_PUSH(queue, struct generic_driver_tree_node_t*, current_node->next_peer);
            if (!queue.ptr) return 0;
        }

        if (current_node->first_child)
        {
            QUEUE_PUSH(queue, struct generic_driver_tree_node_t*, current_node->first_child);
            if (!queue.ptr) return 0;
        }
    }

    QUEUE_FINI(queue);
    return found;
}

const struct generic_driver_tree_node_t* driver_get_by_id(
    struct generic_driver_tree_node_t** root, 
    void* match_id
)
{
    struct generic_driver_tree_node_t* best=NULL;
    usize items_found = 
    driver_get_all_customized(root, found_list, driver_match_id, match_id);

    best = driver_choose_best_node(found_list, items_found);

    memset(found_list, 0, 
        sizeof(struct generic_driver_tree_node_t*)*MAX_DRIVERS_FOUND);
    return best;
}

void driver_set_id_data(
    struct generic_driver_tree_node_t* driver,
    ...
)
{
    va_list args;
    va_start(args, driver);
    switch (driver->id_type)
    {
        case DRIVER_ID_TYPE_HWID:
            driver->id.hwid.device = (u16)va_arg(args, int);
            driver->id.hwid.vendor = (u16)va_arg(args, int);
            break;
        case DRIVER_ID_TYPE_INTERNAL:
        {
            const char* src = va_arg(args, const char*);
            usize len = strlen(src);
            driver->id.compatible = driver_alloc(driver, len, DRIVER_ALLOC_FLAG_HEAP, 0);
            if (!driver->id.compatible)
            driver_log_state(driver, DRIVER_LOG_ERROR, "Out of memory to allocate CID\n");
            memcpy(driver->id.compatible, src, len);
            break;
        }
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown identification method\n");
            break;
    }
    va_end(args);
}

void driver_clear_id_data(
    struct generic_driver_tree_node_t* driver
)
{
    switch (driver->id_type)
    {
        case DRIVER_ID_TYPE_INTERNAL:
            driver_free(driver, driver->id.compatible);
            driver->id.compatible = NULL;
            break;
        case DRIVER_ID_TYPE_HWID:
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR,
                "Unknown identification method");
            break;
    }

}

bool driver_match_id(
    struct generic_driver_tree_node_t* driver,
    void* id_data
)
{
    switch (driver->id_type)
    {
        case DRIVER_ID_TYPE_HWID:
        {
            struct hwid_t* client_hwid = (struct hwid_t*)id_data;
            return driver->id.hwid.device == client_hwid->device
            && driver->id.hwid.vendor == client_hwid->vendor;
        }
        case DRIVER_ID_TYPE_INTERNAL:
            return strcmp(driver->id.compatible, (const char*)id_data) == 0;
        default:
            return false;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static struct generic_driver_tree_node_t* driver_choose_best_node(
    struct generic_driver_tree_node_t* found_list[],
    usize items_found
)
{
    if (items_found == 0) return NULL;

    struct generic_driver_tree_node_t* best=NULL;
    
    for (usize i=0;i<items_found;i++)
    {
        struct generic_driver_tree_node_t* to_check =
        found_list[i];

        if (!best)
        {
            best = to_check;
            continue;
        }

        if (best->mode == DRIVER_MODE_KRNL
        && to_check->mode == DRIVER_MODE_USER)
        {
            best = to_check;
            continue;
        }

        if (best->mode == DRIVER_MODE_USER
        && to_check->mode == DRIVER_MODE_KRNL)
            continue;
        
        if (to_check->priority > best->priority)
        {
            best = to_check;
            continue;
        }
    }

    return best;
}