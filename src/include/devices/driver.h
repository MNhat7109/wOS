#pragma once
#include <stdbool.h>
#include <libk/stdint.h>
#include <libk/stdatomic.h>

#include <devices/mmio.h>
#include <devices/pio.h>
#include <devices/driver_defs.h>

struct generic_driver_tree_node_t;

struct generic_driver_resource_t
{
    generic_driver_res_type_t res_type;
    
    union res_data_t
    {
        struct generic_driver_tree_node_t* dependency;
        struct int_info_t {u8 int_line, msi_vector;} 
        __attribute__((packed)) interrupt_info;
        struct mmio_info_t mmio_info;
        struct pio_info_t pio_info;
        u8 dma_channel;
    } resource;

    struct generic_driver_resource_t* next;
};

struct generic_driver_ops_t
{
    void (*config)(struct generic_driver_tree_node_t*);
    void (*probe)(struct generic_driver_tree_node_t*);
    void (*disable)(struct generic_driver_tree_node_t*);
};

struct hwid_t {u16 vendor, device;} __attribute__((packed));

struct generic_driver_tree_node_t
{
    const char* name;
    generic_driver_state_t state;
    ATOMIC_TYPE(u32) refcount;

    generic_driver_id_type_t id_type;
    union
    {
        struct hwid_t hwid;
        const char* compatible;
    } id;

    generic_driver_bus_type_t bus_type;
    
    u8 priority;
    generic_driver_mode_t mode;

    u32 iid;

    void* additionals;
    struct generic_driver_resource_t* resource_list;
    struct generic_driver_tree_node_t* parent;
    struct generic_driver_tree_node_t* next_peer;
    struct generic_driver_tree_node_t* first_child;

    struct generic_driver_ops_t* ops;
};

typedef enum
{
    DRIVER_LOG_ERROR,
    DRIVER_LOG_WARN,
    DRIVER_LOG_NOTICE,
} log_state_t;

extern struct generic_driver_tree_node_t* driver_forest;
typedef const struct generic_driver_ops_t* (*driver_callback_t)();
#define GET_DEV_OPS(type, node) \
    ((struct type##_driver_ops_t *)((node)->ops))

usize driver_get_all_customized(
    struct generic_driver_tree_node_t** root, 
    struct generic_driver_tree_node_t** found_list,
    bool (*id_method)(struct generic_driver_tree_node_t*, void*),
    void* id_data
);

const struct generic_driver_tree_node_t* driver_get_by_id(
    struct generic_driver_tree_node_t** root, 
    void* match_id
);
bool driver_match_id(
    struct generic_driver_tree_node_t* driver,
    void* id_data
);
void driver_set_id_data(
    struct generic_driver_tree_node_t* driver,
    ...
);
void driver_clear_id_data(
    struct generic_driver_tree_node_t* driver
);


void driver_log_state(struct generic_driver_tree_node_t* driver, log_state_t level, const char* fmt, ...);
bool driver_run(struct generic_driver_tree_node_t* driver);
bool driver_load_ops(
    struct generic_driver_tree_node_t* driver,
    driver_callback_t callback
);
bool driver_terminate(struct generic_driver_tree_node_t* driver);


void* driver_alloc(
    struct generic_driver_tree_node_t* driver, 
    usize size, 
    generic_driver_alloc_flags_t alloc_flags,
    usize mmu_specific
);
void driver_free(struct generic_driver_tree_node_t* driver, void* ptr);

struct generic_driver_tree_node_t* driver_add_to_tree(
    struct generic_driver_tree_node_t** root, 
    struct generic_driver_tree_node_t* parent, 
    generic_driver_id_type_t id_type,
    generic_driver_bus_type_t bus_type,
    u8 priority,
    generic_driver_mode_t mode
);
void driver_remove_from_tree(
    struct generic_driver_tree_node_t** root,
    struct generic_driver_tree_node_t* driver
);

struct generic_driver_resource_t* driver_request_resource(
    struct generic_driver_tree_node_t* driver,
    generic_driver_res_type_t res_type
);
bool driver_untie_resource(
    struct generic_driver_tree_node_t* driver,
    struct generic_driver_resource_t* resource
);

void driver_set_res_data(
    struct generic_driver_tree_node_t* driver,
    struct generic_driver_resource_t* resource,
    ...
);
void driver_find_res_data(
    struct generic_driver_tree_node_t* driver,
    generic_driver_res_type_t type,
    void* data_out,
    void* criteria
);

void driver_ref(struct generic_driver_tree_node_t* driver);
void driver_deref(struct generic_driver_tree_node_t* driver);