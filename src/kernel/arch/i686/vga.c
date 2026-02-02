#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/console.h>
#include <stdint.h>
#include <kernel/arch/i686/io.h>

volatile char* vram = (volatile char*)0xB8000;
u32 vga_height = 80, vga_width = 25;

extern console_backend_t* video_backend;
extern u32 video_cols, video_rows, video_fg, video_bg;

void vga_clear();
void vga_putchar(char ch);
void vga_set_cursor(u32 x, u32 y);
void vga_set_color(u32 fg, u32 bg);

console_backend_t vga_backend = {
    .clear = &vga_clear,
    .putchar = &vga_putchar,
    .set_cursor = &vga_set_cursor,
    .set_color = &vga_set_color
};

void video_fallback_init()
{
    usize vram_pages = mmu_byte_to_4k_pages(0x8000);
    mmu_mmapn((paddr_t)vram, vram_pages, MMU_PG_ATTR_PCD | MMU_PG_ATTR_RW, MMU_FLAG_MAP_ID);

    video_backend = &vga_backend;
}

void vga_clear()
{
    for (u32 y =0;y<vga_height;y++)
    {
        for (u32 x; x<vga_width;x++)
        {
            vram[2*(y*vga_width+x)] = ' ';
            vram[2*(y*vga_width+x)+1] = (video_bg&0x7) << 4 | (video_fg&0xF);
        }
    }
}

void vga_putchar(char ch)
{
    vram[2*(video_rows*vga_width+video_cols)] = ch;
    vram[2*(video_rows*vga_width+video_cols)+1] = (video_bg&0x7) << 4 | (video_fg&0xF);
}

// void vga_putpix(u32 x, u32 y, u32 color){}

// void vga_scroll(u32 lines)
// {
//     for (int i = lines; i < vga_height; i++)
//     {
//         for (int j=0;j<vga_width;j++)
//         {
//             u8 color = vga_getcolor(j, i-lines);
//             u8 fg = color & 0xF, bg = (color >> 4) & 0x7;
//             vga_putchar(j, i, vga_getchar(j, i-lines), fg, bg);
//         }
//     }

//     for (int i=vga_height-lines;i<vga_height;i++)
//     {
//         for (int j=0;j<vga_width;j++)
//         {
//             vga_putchar(j, i, '\0', current_fg, current_bg);
//         }
//     }
// }

void vga_set_cursor(u32 x, u32 y)
{
    u32 pos = y*vga_width+x;
    outb(0x3D4, 0x0F); // Write to register 0x0F to set the highest 8 bits of cursor location.
    // Set the data on port 0x3D5
    outb(0x3D5, ((pos&0xF)>>8));
    outb(0x3D4, 0x10); // Same thing for lower 8 bits
    outb(0x3D5, (pos>>8));
}

void vga_set_color(u32 fg, u32 bg)
{
    video_bg = bg;
    video_fg = fg;
}