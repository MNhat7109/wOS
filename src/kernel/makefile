KERNEL_COLORSTART = \e[37m
KERNEL_COLOREND = \e[0m

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99
TARGET_LDFLAGS += -T linker.ld -nostdlib

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC_DIR = $(shell pwd)
BUILD_DIR = $(shell echo $(subst src,build, $(SRC_DIR)) | sed 's/^[ ]*//')

ASMSRCS = $(call rwildcard, $(SRC_DIR), *.asm)
ASMOBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/asm/%.o, $(ASMSRCS))

CSRCS = $(call rwildcard, $(SRC_DIR), *.c)
COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/c/%.o, $(CSRCS))

BIN = $(BUILD_DIR)/kernel.elf

.PHONY: run

run: link 

link: $(ASMOBJS) $(COBJS)
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Linking files...\n"
	$(TARGET_CC) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $(BIN) $^ -lgcc

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.asm
	@printf "$(KERNEL_COLORSTART)[KERNE:]$(KERNEL_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^