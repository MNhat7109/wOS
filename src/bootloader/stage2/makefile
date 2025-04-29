STAGE2_COLORSTART = \e[31m
STAGE2_COLOREND = \e[0m

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -Oz -Wall -Wextra -ffreestanding -nostdlib -std=c99
TARGET_LDFLAGS += -T linker.ld -nostdlib

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC_DIR = $(shell pwd)
BUILD_DIR = $(shell echo $(subst src,build, $(SRC_DIR)) | sed 's/^[ ]*//')

ASMSRCS = $(call rwildcard, $(SRC_DIR), *.asm)
ASMOBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/asm/%.o, $(ASMSRCS))

CSRCS = $(call rwildcard, $(SRC_DIR), *.c)
COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/c/%.o, $(CSRCS))

BIN = $(BUILD_DIR)/stage2.bin

.PHONY: run

run: link 

link: $(ASMOBJS) $(COBJS)
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Linking files...\n"
	$(TARGET_CC) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/stage2.map -o $(BIN) $^ -lgcc

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.asm
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^