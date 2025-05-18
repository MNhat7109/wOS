#include "elf.h"
#include "../disk/disk.h"
#include "../fat/fat.h"
#include "../string/string.h"

#define ELF_MAG ("\x7f" "ELF")

#define EHDR_ARCH_X86    1
#define EHDR_ARCH_X86_64 2

#define EHDR_ENDIAN_LIL  1
#define EHDR_ENDIAN_BIG  2

#define EHDR_TYPE_RELOC  1
#define EHDR_TYPE_EXEC   2
#define EHDR_TYPE_SHARED 3
#define EHDR_TYPE_CORE   4

#define EHDR_OSABI_SYSV  0

#define EHDR_ISET_UNK     0x00
#define EHDR_ISET_SPARC   0x02
#define EHDR_ISET_X86     0x03
#define EHDR_ISET_MIPS    0x08
#define EHDR_ISET_PPC     0x14
#define EHDR_ISET_ARM     0x28
#define EHDR_ISET_SUPERH  0x2A
#define EHDR_ISET_IA64    0x32
#define EHDR_ISET_X86_64  0x3E
#define EHDR_ISET_AARCH64 0xB7
#define EHDR_ISET_RISC_V  0xF3

#define PHDR_TYPE_NULL 0
#define PHDR_TYPE_LOAD 1
#define PHDR_TYPE_DYN  2
#define PHDR_TYPE_INT  3
#define PHDR_TYPE_NOTE 4

#define PHDR_FLAG_EXECUTABLE 1
#define PHDR_FLAG_WRITABLE   2
#define PHDR_FLAG_READABLE   4

typedef struct
{
    u8 magic[4];
    u8 arch;
    u8 endianness;
    u8 hdr_version;
    u8 os_abi;
    u64 padding;
    u16 type;
    u16 instruction_set;
    u32 elf_version;
    u32 prog_ent_offset;
    u32 prog_hdr_table_offset;
    u32 sect_hdr_table_offset;
    u32 flags;
    u16 hdr_size;
    u16 phdr_entry_size;
    u16 phdr_entry_cnt;
    u16 shdr_entry_size;
    u16 shdr_entry_cnt;
    u16 shdr_stable_idx;
} __attribute__((packed)) ELF_hdr_t;

typedef struct
{
    u32 type;
    u32 offset;
    u32 virt_addr;
    u32 phys_addr;
    u32 size_in_file;
    u32 size_in_mem;
    u32 flags;
    u32 align;
} __attribute__((packed)) ELF_p_hdr_t;


u8* elf_buffer = (u8*)0x12000;
u8* krnl_load_buffer = (u8*)0x22000;

bool ELF_load_file(const char* path, void** entry_point)
{
    u32 pos = 0, read;

    FAT_file_t* fp = FAT_open(path);
    if (!fp) return false;
    read = FAT_read(fp, sizeof(ELF_hdr_t), (void*)elf_buffer);
    if (read != sizeof(ELF_hdr_t)) return false;
    pos+=read;
    // ELF Header Check
    bool ok = true;
    ELF_hdr_t* ehdr = (ELF_hdr_t*)elf_buffer;
    ok = ok && (memcmp(ehdr->magic, ELF_MAG, 4)==0);
    ok = ok && (ehdr->arch == EHDR_ARCH_X86);
    ok = ok && (ehdr->endianness == EHDR_ENDIAN_LIL);
    ok = ok && (ehdr->hdr_version == 1);
    ok = ok && (ehdr->elf_version == 1);
    ok = ok && (ehdr->type == EHDR_TYPE_EXEC);
    ok = ok && (ehdr->instruction_set = EHDR_ISET_X86);
    if (!ok) return ok;

    *entry_point = (void*)ehdr->prog_ent_offset;

    // ELF Program Header Load
    u32 phdr_offset = ehdr->prog_hdr_table_offset;
    u32 phdr_ent_size = ehdr->phdr_entry_size;
    u32 phdr_ent_cnt = ehdr->phdr_entry_cnt;
    u32 phdr_size = phdr_ent_cnt*phdr_ent_size;

    pos += FAT_read(fp, phdr_offset-pos, elf_buffer); // Skip

    read = FAT_read(fp, phdr_size, elf_buffer);
    if (read != phdr_size) return false;
    pos+=read;

    FAT_close(fp);

    for (u32 i=0;i<phdr_ent_cnt;i++)
    {
        ELF_p_hdr_t* phdr = (ELF_p_hdr_t*)(elf_buffer+i*phdr_ent_size);
        if (phdr->type != PHDR_TYPE_LOAD) continue;
        u8* addr = (u8*)phdr->virt_addr;
        memset(addr, 0, phdr->size_in_mem);
        fp = FAT_open(path);

        FAT_seek(fp, phdr->offset);

        while (phdr->size_in_file > 0)
        {
            u32 cnt = phdr->size_in_file<0x10000?phdr->size_in_file:0x10000;
            read = FAT_read(fp, cnt, krnl_load_buffer);
            if (read != cnt) return false;
            phdr->size_in_file-=read;
            memcpy(addr, krnl_load_buffer, read);
            addr+=read;
        }
        FAT_close(fp);
    }
    return true;
}