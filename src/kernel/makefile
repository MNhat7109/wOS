KERNEL_COLORSTART = \e[37m
KERNEL_COLOREND = \e[0m

SRC_DIR = $(shell pwd)
BUILD_DIR = $(shell echo $(subst src,build, $(SRC_DIR)) | sed 's/^[ ]*//')
ARCH_DIR = $(SRC_DIR)/arch

TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99
TARGET_LDFLAGS += -T linker.ld -nostdlib

ifeq ($(TARGET), i686-elf)
TARGET_ASMFLAGS += -f elf32
ARCH_SUBDIR = x86/i386
else ifeq ($(TARGET), x86_64-elf)
TARGET_ASMFLAGS += -f elf64
ARCH_SUBDIR = x86/x86_64
else
$(error Unsupported TARGET: $(TARGET))
endif

BIN = $(BUILD_DIR)/kernel.elf

ASMSRCS = $(shell find $(SRC_DIR) -path "$(ARCH_DIR)/*" -prune -o -type f -name "*.asm" -print)
CSRCS = $(shell find $(SRC_DIR) -path "$(ARCH_DIR)/*" -prune -o -type f -name "*.c" -print)

ASMSRCS += $(shell find $(ARCH_DIR)/$(ARCH_SUBDIR) -type f -name "*.asm" -print)
CSRCS += $(shell find $(ARCH_DIR)/$(ARCH_SUBDIR) -type f -name "*.c" -print)

ASMOBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/asm/%.o, $(ASMSRCS))
COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/c/%.o, $(CSRCS))

CLIB_ORDER = libk libarch libdevices
CLIBS = $(addprefix $(SHARED_DIR)/, $(addsuffix .a,$(CLIB_ORDER)))

.PHONY: run lib_build link

run: lib_build link

link: $(ASMOBJS) $(COBJS) $(CLIBS)
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Linking files...\n"
	$(TARGET_CC) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map \
	-o $(BIN) \
	-L$(SHARED_DIR) \
	$(filter-out $(CLIBS), $^) \
	-Wl,--start-group $(addprefix -l:,$(CLIB_ORDER:=.a)) -Wl,--end-group \
	-lgcc

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.asm
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Assembling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^ -I$(INC_DIR)

$(CLIBS): lib_build

lib_build:
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling libraries...\n"
	$(MAKE) -C $(LIBSRC_DIR)/