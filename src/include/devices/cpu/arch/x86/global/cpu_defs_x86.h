#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

typedef struct x86_cpu_flags_t
{  
    u8 xapic : 1;
    u8 x2apic : 1;
    u8 x2apic_on : 1;
    u8 topology_0x1F : 1;
    u8 topology_0xB : 1;
    u8 reserved : 3;
} __attribute__((packed)) x86_cpu_flags_t;

struct generic_driver_tree_node_t;
struct cio_layer_t;
typedef cio_layer_t cio_layer_t;
struct x86_cpu_flags_t;
typedef struct x86_cpu_flags_t x86_cpu_flags_t;

typedef struct x86_cpu_global_additional_param_t
{
    struct generic_driver_tree_node_t* mps_driver_node,
    * madt_driver_node; 
    x86_cpu_flags_t* features;
    u16 lx2apic_msr; // ONLY used if x2APIC is supported, reserved otherwise.
    const cio_layer_t* cpu_io;
} x86_cpu_global_additional_param_t;

typedef struct x86_cpu_core_param_t
{
    const x86_cpu_global_additional_param_t* base;
    u32 proc_id;
    u32 acpi_proc_id;
    u32 lapic_id;
    u32 flags;
    bool is_bsp;
} x86_cpu_core_param_t;

typedef struct x86_cpu_global_param_t
{
    x86_cpu_flags_t global_cpu_flags;
    struct generic_driver_tree_node_t* acpi_driver_node;
    x86_cpu_global_additional_param_t export_params;
} x86_cpu_global_param_t;


///////////////////////////////////////////////////////////////////////


typedef struct mpfp_t
{
    char signature[4]; // Always "_MP_"
    u32 paddr; // The physical address of the MP Table
    u8 length; // Length in 16-byte chunks (Always 1)
    u8 spec_revision; // 1 for rev 1.1 or 4 for 1.4
    u8 checksum; // 8-bit checksum
    u8 features[5];
} __attribute__((packed)) mpfp_t;

typedef struct mpct_hdr_t
{
    char signature[4]; // Always "PCMP"
    u16 length; // Total length of the header+entries
    u8 spec_revision; // 1 for rev 1.1 or 4 for 1.4
    u8 checksum; // Once-again, 8-bit checksum
    char oem_id[8];
    char product_id[12];
    u32 oem_table_phys;
    u16 oem_table_size;
    u16 entry_count; // Number of entries follow the header
    u32 lapic_addr;

    struct mpct_ext_t
    {
        u16 ext_length;
        u8 ext_checksum;
        u8 _reserved;
    } __attribute__((packed)) ext_fields;
} __attribute__((packed)) mpct_hdr_t;