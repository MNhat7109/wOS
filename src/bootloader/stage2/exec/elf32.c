#include "elf.h"
#include "../stdint.h"
#include "../stdio.h"
#include "../string/string.h"
#include "../errno.h"

#include "../fs/fs.h"

typedef struct elf32_ehdr_t
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
} __attribute__((packed)) elf32_ehdr_t;

typedef struct elf32_phdr_t
{
    u32 type;
    u32 offset;
    u32 virt_addr;
    u32 phys_addr;
    u32 size_in_file;
    u32 size_in_mem;
    u32 flags;
    u32 align;
} __attribute__((packed)) elf32_phdr_t;

static struct
{
    u8 elf_header_buffer[sizeof(elf32_ehdr_t)];
    u8 elf_temp_buffer[512];
} elf32_data;

int elf32_check_ehdr(fs_t* fs, file_t* file, void** entry_point);
int elf32_load_phdr(fs_t* fs, file_t* file, elf32_ehdr_t* hdr);

int elf_load_32(fs_t* fs, const char* path, void** entry_point)
{
    if (!fs) return -EINVAL;
    if (!fs->ops) return -EINVAL;

    file_t* fp; int status; u32 pos = 0;
    status = fs->ops->open(fs, path, &fp);
    if (status < 0) return status;

    status = elf32_check_ehdr(fs, fp, entry_point);
    if (status < 0) return status;

    status = elf32_load_phdr(fs, fp, (elf32_ehdr_t*)elf32_data.elf_header_buffer);
    if (status < 0) return status;

    fs->ops->close(fp);

    kdebugf(DEBUG_INFO, MODULE_EXEC_ELF,"%s loaded at entry point 0x%x\n", path, *entry_point);
    return 0;
}

int elf32_check_ehdr(fs_t* fs, file_t* file, void** entry_point)
{
    if (!fs) return -EINVAL;
    if (!fs->ops) return -EINVAL;
    if (!file) return -EINVAL;

    u32 read_count = fs->ops->read(file, sizeof(elf32_ehdr_t), 
    elf32_data.elf_header_buffer);
    if (read_count != sizeof(elf32_ehdr_t)) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_EXEC_ELF, "Cannot read ELF header\n");
        return -EASSERT;
    }

    elf32_ehdr_t* ehdr = (elf32_ehdr_t*)elf32_data.elf_header_buffer;
    
    u8 ok = 1;
    ok = ok && (memcmp(ehdr->magic, ELF_MAG, 4)==0);
    ok = ok && (ehdr->arch == EHDR_ARCH_X86);
    ok = ok && (ehdr->endianness == EHDR_ENDIAN_LIL);
    ok = ok && (ehdr->hdr_version == 1);
    ok = ok && (ehdr->elf_version == 1);
    ok = ok && (ehdr->type == EHDR_TYPE_EXEC);
    ok = ok && (ehdr->instruction_set == EHDR_ISET_X86);

    if (!ok) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_EXEC_ELF, "Invalid ELF header\n");
        return -EASSERT;
    }

    *entry_point = (void*)ehdr->prog_ent_offset;
    return 0;
}

int elf32_load_phdr(fs_t* fs, file_t* file, elf32_ehdr_t* hdr)
{
    if (!fs) return -EINVAL;
    if (!fs->ops) return -EINVAL;
    if (!file) return -EINVAL;
    if (!hdr) return -EINVAL;

    u32 phdr_offset = hdr->prog_hdr_table_offset;
    u32 phdr_ent_size = hdr->phdr_entry_size;
    u32 phdr_ent_cnt = hdr->phdr_entry_cnt;
    u32 phdr_size = phdr_ent_cnt*phdr_ent_size;

    
    int status = fs->ops->seek(file, phdr_offset);
    if (status < 0) return status;

    u32 remaining_entries = phdr_ent_cnt;
    u32 max_entries = 512 / phdr_ent_size;

    
    while (remaining_entries > 0)
    {
        u32 read_count = (remaining_entries < max_entries)?remaining_entries:max_entries;
        u32 byte_count = read_count*phdr_ent_size;
        
        u32 read = fs->ops->read(file, byte_count, elf32_data.elf_temp_buffer);
        if (read != byte_count) return -EASSERT;
        
        for (u32 i=0;i<read_count;i++)
        {
            elf32_phdr_t* phdr = (elf32_phdr_t*)(elf32_data.elf_temp_buffer+i*phdr_ent_size);
            
            if (phdr->type != PHDR_TYPE_LOAD) continue;
            
            u8* addr = (u8*)phdr->phys_addr;
            memset(addr, 0, phdr->size_in_mem);
            
            status = fs->ops->seek(file, phdr->offset);
            if (status < 0) return status;
            
            read = fs->ops->read(file, phdr->size_in_file, addr);
            if (read != phdr->size_in_file) return -EASSERT;
            
            if (status < 0) return status;
        }
        
        remaining_entries -= read_count;
    }
    return 0;
}