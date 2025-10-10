#pragma once
#include <libk/stdint.h>
#include <libk/bitmap/bitmap.h>

typedef struct buddy_metadata_t
{
    struct buddy_meta_info_t
    {
        u8 level : 6;
        u8 check : 2;
    } __attribute__((packed)) buddy_info;
    void* user_data;
} buddy_metadata_t;

typedef struct buddy_alloc_zone_t
{
    void* buddy_address;
    usize buddy_total_size;
    usize buddy_max_level;
    usize buddy_granularity;

    bitmap_t buddy_bmp;

    usize meta_item_count;
    buddy_metadata_t* meta_start;
} buddy_alloc_zone_t;

void buddy_alloc_init(buddy_alloc_zone_t* zone, void* address,
    usize length, usize granularity);
void buddy_alloc_destroy(buddy_alloc_zone_t* zone);
void* buddy_alloc_request(buddy_alloc_zone_t* zone, usize size);
void buddy_alloc_free(buddy_alloc_zone_t* zone, void* ptr);