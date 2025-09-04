#include <devices/driver.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <libk/stdatomic.h>
#include <stdarg.h>

#include <libk/containers/queue.h>
#include <libk/containers/stack.h>

#define MAX_DRIVERS_FOUND 128

static u8 driver_clamp_priority(
    generic_driver_mode_t mode,
    u8 raw_priority
);

static u8 driver_request_priority(
    generic_driver_mode_t mode,
    generic_driver_bus_type_t bus_type
);

struct generic_driver_tree_node_t* driver_add_to_parent(
    struct generic_driver_tree_node_t* parent, 
    generic_driver_id_type_t id_type,
    generic_driver_bus_type_t bus_type,
    u8 requested_priority,
    generic_driver_mode_t mode
)
{
    // Create a new node
    struct generic_driver_tree_node_t* new_node = 
    kmalloc(sizeof(struct generic_driver_tree_node_t));
    if (!new_node)
        return NULL;

    // Populate that thing
    new_node->parent = parent;
    new_node->bus_type = bus_type;
    new_node->id_type = id_type;
    ATOMIC_STORE(new_node->refcount, 1);
    new_node->next_peer = NULL;
    new_node->first_child = NULL;
    new_node->mode = mode;
    new_node->state = DRIVER_STATE_UNPROBED;

    // Always clamp the priority number first before doing anything
    requested_priority = driver_clamp_priority(mode, requested_priority);

    new_node->priority = 
    requested_priority ? 
    requested_priority :
    driver_request_priority(mode, bus_type);

    // If there's no parent, it's likely a root
    if (parent)
    {
        if (!parent->first_child) parent->first_child = new_node;
        else
        {
            struct generic_driver_tree_node_t* child_link = parent->first_child;
            while (child_link->next_peer)
                child_link = child_link->next_peer;

            child_link->next_peer = new_node;
        }
    }

    return new_node;
}

struct generic_driver_tree_node_t* driver_add_to_tree(
    struct generic_driver_tree_node_t** root, 
    struct generic_driver_tree_node_t* parent, 
    generic_driver_id_type_t id_type,
    generic_driver_bus_type_t bus_type,
    u8 priority,
    generic_driver_mode_t mode
)
{
    struct generic_driver_tree_node_t* new_node = 
    driver_add_to_parent(parent, id_type, bus_type, mode, priority);
    if (!new_node) return NULL;

    // Case 1: First root
    if (!parent && !*root)
    {
        *root = new_node;
    }
    // Case 2: Multiple roots, append as linked list
    else if (!parent && *root)
    {
        struct generic_driver_tree_node_t* peer_root = *root;
        while (peer_root->next_peer)
            peer_root = peer_root->next_peer;

        peer_root->next_peer = new_node;
    }

    return new_node;
}

void driver_remove_from_tree(
    struct generic_driver_tree_node_t** root,
    struct generic_driver_tree_node_t* driver
)
{
    if (!driver) return;

    // Unlink the node
    // But if the refcount is getting high, we skip that
    if (ATOMIC_LOAD(driver->refcount) == 1)
    {
        if (driver->parent)
        {
            // Unlink at the parent's level
            struct generic_driver_tree_node_t** child_link = &driver->parent->first_child;
    
            while (*child_link && *child_link != driver)
                child_link = &(*child_link)->next_peer;
            
            if (*child_link == driver) *child_link = driver->next_peer;
        }
        else
        {
            // Unlink at root level
            struct generic_driver_tree_node_t** root_link = root;
            
            while (*root_link && *root_link != driver)
                root_link = &(*root_link)->next_peer;
    
            if (*root_link == driver) *root_link = driver->next_peer;
        }
    }

    // DFS and deref or pull the plug on the children, then the node itself
    STACK(struct generic_driver_tree_node_t*) stack;
    STACK_INIT(stack);
    if (!stack.ptr) return;
    STACK_PUSH(stack, struct generic_driver_tree_node_t*, driver);

    while (!STACK_EMPTY(stack))
    {
        struct generic_driver_tree_node_t* current_node =
        STACK_TOP(stack, struct generic_driver_tree_node_t*);

        STACK_POP(stack);

        if (ATOMIC_LOAD(current_node->refcount) > 1)
        {
            driver_deref(current_node);
            continue;
        }

        // Push the children
        struct generic_driver_tree_node_t* child = current_node->first_child;
        while (child)
        {
            STACK_PUSH(stack, struct generic_driver_tree_node_t*, child);
            child = child->next_peer;
        }
        
        // TODO: Replace with CAS once multi-core refcounting is needed

        driver_clear_id_data(current_node);
        kfree(current_node);
        current_node = NULL;
    }

    STACK_FINI(stack);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static u8 driver_request_priority(
    generic_driver_mode_t mode,
    generic_driver_bus_type_t bus_type
)
{
    u8 suggested = 0;
    switch (mode)
    {
        case DRIVER_MODE_USER:
            suggested = 200;
            break;
        case DRIVER_MODE_KRNL:
            switch (bus_type)
            {
                case DRIVER_BUS_TYPE_USB:
                    suggested = 70; // USB Stack
                    break;
                case DRIVER_BUS_TYPE_ACPI:
                case DRIVER_BUS_TYPE_CPU:
                    suggested = 80; // ACPI tables
                    break;
                case DRIVER_BUS_TYPE_PCI:
                    suggested = 90; // Recently-discovered PCI bus controllers
                    break;
                default:
                    suggested = 100; // For standalone generic drivers
                    break;
            }
            break;
        default:
            break;
    }

    return suggested;
}

static u8 driver_clamp_priority(
    generic_driver_mode_t mode,
    u8 raw_priority
)
{
    switch (mode)
    {
        case DRIVER_MODE_USER:
            // Force into [128, 255] by masking low bits
            raw_priority = 0x80 | (raw_priority & 0x7F);
            break;
        case DRIVER_MODE_KRNL:
            // Force into [0, 127]
            raw_priority &= 0x7F;
            break;        
        default:
            // Unknown mode, reduce it to zero
            raw_priority=0;
            break;
    }

    return raw_priority;
}