export TARGET?=i686
export SMALL_SCREEN?=0
export QEMU_TARGET=$(if $(filter i686,$(TARGET)),i386,$(TARGET))
export OSNAME=wOS

export ASM=nasm
export PYTHON=python3
export ASMFLAGS=
export CC=gcc
export CFLAGS=
export LD=ld
export LDFLAGS=
export AR=ar
export RANLIB=ranlib
export QEMU=qemu-system-$(QEMU_TARGET)

export TARGET_CC=$(TARGET)-elf-$(CC)
export TARGET_CFLAGS=
export TARGET_ASM=$(ASM)
export TARGET_ASMFLAGS=
export TARGET_LD=$(TARGET)-elf-$(LD)
export TARGET_LDFLAGS=
export TARGET_AR=$(TARGET)-elf-$(AR)
export TARGET_RANLIB=$(TARGET)-elf-$(RANLIB)

export SRC_DIR = $(abspath src)
export BUILD_DIR = $(abspath build)
export SCRIPT_DIR = $(abspath build_scripts)
export LIBC_DIR = $(abspath libc)
export LIBDS_DIR = $(abspath libds)
export BASE_DIR = $(abspath rootfs)

export BOOT_BUILD_DIR = $(BUILD_DIR)/bootloader
export KERNEL_BUILD_DIR = $(BUILD_DIR)/kernel
export LIBC_BUILD_DIR = $(BUILD_DIR)/libc
export LIBDS_BUILD_DIR = $(BUILD_DIR)/libds

export ROOT_DIR = $(BASE_DIR)/root
export INC_DIR = $(ROOT_DIR)/usr/include