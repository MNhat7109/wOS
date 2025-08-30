#pragma once

typedef enum 
{
    DRIVER_ALLOC_FLAG_ID,
    DRIVER_ALLOC_FLAG_HEAP,
} generic_driver_alloc_flags_t;

typedef enum 
{
    DRIVER_STATE_FAILED,
    DRIVER_STATE_READY,
    DRIVER_STATE_PROBED,
    DRIVER_STATE_UNPROBED,
    DRIVER_STATE_DISABLED,
} generic_driver_state_t;

typedef enum
{
    DRIVER_ID_TYPE_HWID,
    DRIVER_ID_TYPE_INTERNAL,
    DRIVER_ID_TYPE_OTHER = 0xFF
} generic_driver_id_type_t;

typedef enum
{
    DRIVER_MODE_KRNL,
    DRIVER_MODE_USER,
} generic_driver_mode_t;

typedef enum
{
    DRIVER_BUS_TYPE_ACPI,
    DRIVER_BUS_TYPE_CPU,
    DRIVER_BUS_TYPE_PCI,
    DRIVER_BUS_TYPE_USB,
    DRIVER_BUS_TYPE_STANDALONE,
    DRIVER_BUS_TYPE_UNKNOWN = 0xFF
} generic_driver_bus_type_t;

typedef enum
{
    DRIVER_RES_TYPE_DEPENDENCY,
    DRIVER_RES_TYPE_INTLINE,
    DRIVER_RES_TYPE_MSI,
    DRIVER_RES_TYPE_MMIO,
    DRIVER_RES_TYPE_PIO,
    DRIVER_RES_TYPE_DMA,
    DRIVER_RES_TYPE_UNKNOWN = 0xFF
} generic_driver_res_type_t;