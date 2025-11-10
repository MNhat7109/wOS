KERNEL_COLORSTART = \e[37m
KERNEL_COLOREND = \e[0m

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99
TARGET_LDFLAGS += -T linker.ld -nostdlib

SRC_DIR = $(shell pwd)
BUILD_DIR = $(patsubst %/src, %/build, $(SRC_DIR))

CSRCS = $(wildcard $(SRC_DIR)/*.c) 
CSRCS_SUBS = $(wildcard $(SRC_DIR)/*/*.c) 
CSRCS_ARCH = $(wildcard $(SRC_DIR)/$(TARGET)/*.c)

COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/c/%.o, $(CSRCS))
COBJS += $(patsubst $(SRC_DIR)/%/%.c, $(BUILD_DIR)/c/%/%.o, $(CSRCS_SUBS))
COBJS += $(patsubst $(SRC_DIR)/$(TARGET)/%.c, $(BUILD_DIR)/c/$(TARGET)/%.o, $(CSRCS_ARCH))

ASMSRCS = $(wildcard $(SRC_DIR)/*.asm) 
ASMSRCS_SUBS = $(wildcard $(SRC_DIR)/*/*.asm) 
ASMSRCS_ARCH = $(wildcard $(SRC_DIR)/$(TARGET)/*.asm)

ASMOBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/asm/%.o, $(ASMSRCS))
ASMOBJS += $(patsubst $(SRC_DIR)/%/%.asm, $(BUILD_DIR)/asm/%/%.o, $(ASMSRCS_SUBS))
ASMOBJS += $(patsubst $(SRC_DIR)/$(TARGET)/%.asm, $(BUILD_DIR)/asm/$(TARGET)/%.o, $(ASMSRCS_ARCH))

BIN = $(BUILD_DIR)/kernel.elf

.PHONY: run

run: link 

link: $(ASMOBJS) $(COBJS)
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Linking files...\n"
	$(TARGET_CC) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $(BIN) $^ -lgcc

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.asm
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^