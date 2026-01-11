#pragma once


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

#define MODULE_EXEC_ELF "EXEC_ELF"

typedef struct fs_t fs_t;

int elf_load_32(fs_t* fs, const char* path, void** entry_point);