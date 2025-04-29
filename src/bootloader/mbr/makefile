MBR_COLORSTART = \e[33m
MBR_COLOREND = \e[0m

TARGET_ASMFLAGS += -f bin

SRC_DIR = $(shell pwd)
BUILD_DIR = $(subst src,build, $(SRC_DIR))
SRCS = $(wildcard $(SRC_DIR)/*.asm)
OBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.bin, $(SRCS))

.PHONY: run

run: $(OBJS)

$(BUILD_DIR)/%.bin: $(SRC_DIR)/%.asm
	@printf "$(MBR_COLORSTART)[MBR]$(MBR_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@