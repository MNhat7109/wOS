#pragma once
#include <libk/stdint.h>
#include <stdbool.h>
#include <devices/acpi/acpi_defs.h>

struct generic_driver_tree_node_t;
struct cpu_ipi_request_t;

typedef enum  
{
    CPU_IPI_DEST_CORE,   // specify exact core
    CPU_IPI_DEST_SELF,
    CPU_IPI_DEST_ALL,
    CPU_IPI_DEST_ALL_BUT_SELF,
} cpu_ipi_dest_t;

struct cpu_ipi_request_t 
{
    cpu_ipi_dest_t dest_type;
    struct generic_driver_tree_node_t* dest_core; // only if dest_type == CORE
    u8 vector;                               // interrupt vector (0–255)
    u8 delivery_mode;
    bool trigger_mode;                                   // 0=edge, 1=level
    bool level;                                  // 1=assert, 0=deassert
};

struct cpu_core_ops_t
{
    void (*send_eoi)(u8 int_num);
    u32 (*get_core_id)(
        struct generic_driver_tree_node_t* self
    );
    u32 (*get_arch_id)();
    int (*send_self_ipi)(
        struct generic_driver_tree_node_t* self,
        struct cpu_ipi_request_t* req
    );
};

struct cpu_timer_ops_t
{
    void (*timer_countdown)();
    u64 (*timer_get_freq)(u64 elapsed_ticks, u64 multiplier);
    void (*timer_init)(u32 tick_count, u8 vector);
};

struct cpu_interrupt_ops_t
{
    u32 (*normalize_interrupt_num)(u8 int_num);
    void (*redirect_interrupt)(u8 int_num, u8 vector, u8 core_id);
    void (*rewire_interrupt)(u8 int_num);
    void (*cut_interrupt)(u8 int_num);
    int (*set_spurious_vector)(int vector);
    int (*set_nmi)(int vector, int lapic_id);
};

typedef struct
{
    u8 entry_type;
    u8 record_length;
} __attribute__((packed)) madt_record_entry_hdr_t;

typedef struct madt_t
{
    acpi_sdt_hdr_t table_hdr;
    u32 lapic_addr;
    u32 flags;
} __attribute__((packed)) madt_t;

typedef void (*table_callback_t)(
    struct generic_driver_tree_node_t* cpu_self,
    void* record,
    void* ctx
);