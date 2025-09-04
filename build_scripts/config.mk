export TARGET?=i686-elf
export OSNAME=wOS

export ASM=nasm
export ASMFLAGS=
export CC=gcc
export CFLAGS=
export LD=ld
export LDFLAGS=
export AR=ar
export RANLIB=ranlib

export TARGET_CC=$(TARGET)-$(CC)
export TARGET_CFLAGS=
export TARGET_ASM=$(ASM)
export TARGET_ASMFLAGS=
export TARGET_LD=$(TARGET)-$(LD)
export TARGET_LDFLAGS=
export TARGET_AR=$(TARGET)-$(AR)
export TARGET_RANLIB=$(TARGET)-$(RANLIB)

export SRC_DIR = $(abspath src)
export BUILD_DIR = $(abspath build)
export ROOT_DIR = $(abspath rootfs)
export BOOT_DIR = $(BUILD_DIR)/bootloader
export KRNL_DIR = $(BUILD_DIR)/kernel
export INC_DIR = $(SRC_DIR)/include
export LIBSRC_DIR = $(SRC_DIR)/shared
export SHARED_DIR = $(BUILD_DIR)/libs