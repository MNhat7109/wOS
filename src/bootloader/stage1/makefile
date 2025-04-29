STAGE1_COLORSTART = \e[32m
STAGE1_COLOREND = \e[0m

TARGET_ASMFLAGS += -f bin

SRC_DIR = $(shell pwd)
BUILD_DIR = $(subst src,build, $(SRC_DIR))
SRCS = $(wildcard $(SRC_DIR)/*.asm)
OBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.bin, $(SRCS))

.PHONY: run

run: $(OBJS)

$(BUILD_DIR)/%.bin: $(SRC_DIR)/%.asm
	@printf "$(STAGE1_COLORSTART)[STAGE1]$(STAGE1_COLOREND): Compiling $^\n" 
	@echo Compiling $^ 
	mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) $^ -o $@