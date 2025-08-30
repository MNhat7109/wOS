#pragma once
#include <libk/stdint.h>

typedef enum
{
    MMIO_FLAG_UNCACHEABLE = (1<<0),
    MMIO_FLAG_WRITE_THRU = (1<<1),
} mmio_flags_t;

struct mmio_info_t;

typedef struct mmio_layer_t
{
    u32 (*readl)(struct mmio_info_t* self, u32 offset);
    u64 (*readq)(struct mmio_info_t* self, u32 offset);
    void (*writel)(struct mmio_info_t* self, u32 offset, u32 value);
    void (*writeq)(struct mmio_info_t* self, u32 offset, u64 value);
} mmio_layer_t;

struct mmio_info_t
{
    u64 base, size;
    u32 flags;
    mmio_layer_t* layer;  
};

void mmio_acquire(
    struct mmio_info_t* self, 
    u64 base, 
    u64 size, 
    u32 flags
);
void mmio_release(struct mmio_info_t* self);
