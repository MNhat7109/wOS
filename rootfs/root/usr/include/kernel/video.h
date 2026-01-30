#pragma once
#include <stdint.h>

typedef struct framebuffer_t
{
    u32 base;
    u32 size;
    u32 height, width;
    u32 bpp, pitch;
    u32 xpos, ypos;
} __attribute__((packed)) framebuffer_t;

typedef enum
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_PURPLE,
    VGA_COLOR_BROWN,
    VGA_COLOR_GRAY,
    VGA_COLOR_DARK_GRAY,
    VGA_COLOR_LIGHT_BLUE,
    VGA_COLOR_LIGHT_GREEN,
    VGA_COLOR_LIGHT_CYAN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_PURPLE,
    VGA_COLOR_YELLOW,
    VGA_COLOR_WHITE,
} legacy_vga_color_t;

typedef struct boot_info_t boot_info_t;

typedef struct video_ops_t
{
    void (*clear)(u32 color);
    void (*putchar)(char ch, u32 x, u32 y, u32 fg, u32 bg);
    void (*putpix)(u32 x, u32 y, u32 color);
    void (*scroll)(u32 lines);
    void (*set_cursor)(u32 x, u32 y);
} video_ops_t;

extern video_ops_t* video_ops;
void video_init(boot_info_t* boot_info);