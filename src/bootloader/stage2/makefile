STAGE2_COLORSTART = \e[31m
STAGE2_COLOREND = \e[0m

BOOT_SRC_DIR = $(SRC_DIR)/bootloader
STAGE2_SRC_DIR = $(BOOT_SRC_DIR)/stage2
STAGE2_BUILD_DIR = $(BOOT_BUILD_DIR)/stage2

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99
TARGET_LDFLAGS += -T $(STAGE2_SRC_DIR)/arch/$(TARGET)/linker.ld -nostdlib

CSRCS = $(wildcard $(STAGE2_SRC_DIR)/*.c) 
CSRCS_SUBS = $(wildcard $(STAGE2_SRC_DIR)/*/*.c) 
CSRCS_ARCH = $(wildcard $(STAGE2_SRC_DIR)/arch/$(TARGET)/*.c)

COBJS = $(patsubst $(STAGE2_SRC_DIR)/%.c, $(STAGE2_BUILD_DIR)/c/%.o, $(CSRCS))
COBJS += $(patsubst $(STAGE2_SRC_DIR)/%.c, $(STAGE2_BUILD_DIR)/c/%.o, $(CSRCS_SUBS))
COBJS += $(patsubst $(STAGE2_SRC_DIR)/arch/$(TARGET)/%.c, $(STAGE2_BUILD_DIR)/c/arch/$(TARGET)/%.o, $(CSRCS_ARCH))

ASMSRCS = $(wildcard $(STAGE2_SRC_DIR)/*.asm) 
ASMSRCS_SUBS = $(wildcard $(STAGE2_SRC_DIR)/*/*.asm) 
ASMSRCS_ARCH = $(wildcard $(STAGE2_SRC_DIR)/arch/$(TARGET)/*.asm)

ASMOBJS = $(patsubst $(STAGE2_SRC_DIR)/%.asm, $(STAGE2_BUILD_DIR)/asm/%.o, $(ASMSRCS))
ASMOBJS += $(patsubst $(STAGE2_SRC_DIR)/%.asm, $(STAGE2_BUILD_DIR)/asm/%.o, $(ASMSRCS_SUBS))
ASMOBJS += $(patsubst $(STAGE2_SRC_DIR)/arch/$(TARGET)/%.asm, $(STAGE2_BUILD_DIR)/asm/arch/$(TARGET)/%.o, $(ASMSRCS_ARCH))

BIN = $(STAGE2_BUILD_DIR)/stage2.bin

.PHONY: run

run: link 

link: $(ASMOBJS) $(COBJS)
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Linking files...\n"
	$(TARGET_CC) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/stage2.map -o $(BIN) $^ -lgcc

$(STAGE2_BUILD_DIR)/asm/%.o: $(STAGE2_SRC_DIR)/%.asm
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(STAGE2_BUILD_DIR)/c/%.o: $(STAGE2_SRC_DIR)/%.c
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^

$(STAGE2_BUILD_DIR)/asm/arch/$(TARGET)/%.o: $(STAGE2_SRC_DIR)/arch/$(TARGET)/%.asm
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

$(STAGE2_BUILD_DIR)/c/arch/$(TARGET)/%.o: $(STAGE2_SRC_DIR)/arch/$(TARGET)/%.c
	@printf "$(STAGE2_COLORSTART)[STAGE2]$(STAGE2_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^