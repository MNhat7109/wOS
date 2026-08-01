KERNEL_COLORSTART = \e[37m
KERNEL_COLOREND = \e[0m

KERNEL_SRC_DIR = $(SRC_DIR)/kernel

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -Oz -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -std=c99 -DUSE_KERNEL_TYPES
TARGET_LDFLAGS += -T $(KERNEL_SRC_DIR)/arch/$(TARGET)/linker.ld -nostdlib

CSRCS = $(wildcard $(KERNEL_SRC_DIR)/*.c) 
CSRCS_SUBS = $(wildcard $(KERNEL_SRC_DIR)/*/*.c) 
CSRCS_ARCH = $(wildcard $(KERNEL_SRC_DIR)/arch/$(TARGET)/*.c)

COBJS = $(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_BUILD_DIR)/c/%.o, $(CSRCS))
COBJS += $(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_BUILD_DIR)/c/%.o, $(CSRCS_SUBS))
COBJS += $(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_BUILD_DIR)/c/%.o, $(CSRCS_ARCH))

ASMSRCS = $(wildcard $(KERNEL_SRC_DIR)/*.asm) 
ASMSRCS_SUBS = $(wildcard $(KERNEL_SRC_DIR)/*/*.asm) 
ASMSRCS_ARCH = $(wildcard $(KERNEL_SRC_DIR)/arch/$(TARGET)/*.asm)

ASMOBJS = $(patsubst $(KERNEL_SRC_DIR)/%.asm, $(KERNEL_BUILD_DIR)/asm/%.o, $(ASMSRCS))
ASMOBJS += $(patsubst $(KERNEL_SRC_DIR)/%.asm, $(KERNEL_BUILD_DIR)/asm/%.o, $(ASMSRCS_SUBS))
ASMOBJS += $(patsubst $(KERNEL_SRC_DIR)/%.asm, $(KERNEL_BUILD_DIR)/asm/%.o, $(ASMSRCS_ARCH))

BIN = $(KERNEL_BUILD_DIR)/kernel.elf
LIBC = $(LIBC_BUILD_DIR)/libc.a
LIBDS = $(LIBDS_BUILD_DIR)/libds.a

.PHONY: run

run: libs link

link: $(ASMOBJS) $(COBJS)
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Linking files...\n"
	$(TARGET_CC) \
	$(TARGET_LDFLAGS) \
	-L$(LIBC_BUILD_DIR) \
	-L$(LIBDS_BUILD_DIR) \
	-Wl,-Map=$(KERNEL_BUILD_DIR)/kernel.map \
	-o $(BIN) $^ \
	-l:$(patsubst $(LIBDS_BUILD_DIR)/%,%,$(LIBDS)) \
	-l:$(patsubst $(LIBC_BUILD_DIR)/%,%,$(LIBC)) \
	-lgcc 

$(KERNEL_BUILD_DIR)/asm/%.o: $(KERNEL_SRC_DIR)/%.asm
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n" 
	mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@ 

$(KERNEL_BUILD_DIR)/c/%.o: $(KERNEL_SRC_DIR)/%.c
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling $^\n"
	mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $^ -I$(INC_DIR)

libs: libc libds

libc: $(LIBC)

$(LIBC):
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling libc...\n"
	$(MAKE) -C $(LIBC_DIR)/

libds: $(LIBDS)

$(LIBDS):
	@printf "$(KERNEL_COLORSTART)[KERNEL]$(KERNEL_COLOREND): Compiling libds...\n"
	$(MAKE) -C $(LIBDS_DIR)/