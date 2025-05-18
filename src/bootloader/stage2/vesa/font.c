#include "font.h"
#include "../fat/fat.h"
#include "../stdio.h"

#define PSF1_MAGIC 0x0436
#define PSF1_MODE512 0x01

typedef struct
{
    u16 magic;
    u8 mode;
    u8 charsize;
} __attribute__((packed)) psf1_header_t;

psf1_header_t header;
font_t font;

bool font_init(const char* path)
{
    FAT_file_t* fp = FAT_open(path);
    if (!fp) return false;

    u32 read = FAT_read(fp, sizeof(psf1_header_t), (void*)&header);
    if (read != sizeof(psf1_header_t)) return false;
    
    if (header.magic != PSF1_MAGIC) return false;

    font.height = header.charsize;
    font.width = 8;
    font.glyph = (void*)(&header+sizeof(psf1_header_t));
    if (header.mode == PSF1_MODE512) font.glyph_count = 512;
    else font.glyph_count = 256;
    u32 glyph_size = font.glyph_count*font.height;
    read = FAT_read(fp, glyph_size, font.glyph);
    if (read != glyph_size) return false;
    FAT_close(fp);
    return true;
}