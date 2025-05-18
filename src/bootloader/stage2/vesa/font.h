#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    u8 height, width;
    u16 glyph_count;
    void* glyph;
} __attribute__((packed)) font_t;

extern font_t font;

bool font_init(const char* path);