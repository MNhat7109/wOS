export TARGET=i686
export OSNAME=wOS

export ASM=nasm
export ASMFLAGS=
export CC=gcc
export CFLAGS=
export LD=ld
export LDFLAGS=

export TARGET_CC=$(TARGET)-elf-$(CC)
export TARGET_CFLAGS=
export TARGET_ASM=$(ASM)
export TARGET_ASMFLAGS=
export TARGET_LD=$(TARGET)-elf-$(LD)
export TARGET_LDFLAGS=

export SRC_DIR = src
export BUILD_DIR = build
export BOOT_DIR = $(BUILD_DIR)/bootloader
export KRNL_DIR = $(BUILD_DIR)/kernel