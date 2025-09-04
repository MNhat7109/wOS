SHARED_COLORSTART = \e[36m
SHARED_COLOREND = \e[0m

SRC_DIR := $(shell pwd)
BUILD_DIR := $(shell echo $(subst src,build, $(SRC_DIR)) | sed 's/^[ ]*//')

TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99
TARGET_ASMFLAGS = -f elf32

LIB:= $(SHARED_DIR)/libarch.a

CSRCS:=$(shell find $(SRC_DIR) -type f -name "*.c" -print)
ASMSRCS:=$(shell find $(SRC_DIR) -type f -name "*.asm" -print)

COBJS:=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/c/%.o, $(CSRCS))
ASMOBJS:=$(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/asm/%.o, $(ASMSRCS))

.PHONY: all clean archive

all: clean archive

archive: $(LIB) 

$(LIB): $(COBJS) $(ASMOBJS)
	@printf "$(SHARED_COLORSTART)[LIBARCH-$(TARGET)]$(SHARED_COLOREND): Archiving...\n"
	mkdir -p $(@D)
	$(TARGET_AR) qc $@ $^
	$(TARGET_RANLIB) $@

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	@printf "$(SHARED_COLORSTART)[LIBARCH-$(TARGET)]$(SHARED_COLOREND): Compiling $^\n"
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $^ -o $@ -I$(INC_DIR)

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.asm
	@printf "$(SHARED_COLORSTART)[LIBARCH-$(TARGET)]$(SHARED_COLOREND): Assembling $^\n"
	@mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

clean:
	rm -rf $(LIB)
	rm -rf $(ASMOBJS) $(COBJS)