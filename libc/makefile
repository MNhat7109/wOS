SHARED_COLORSTART = \e[36m
SHARED_COLOREND = \e[0m

LIBNAME=libc

CSRCS=$(wildcard $(LIBC_DIR)/*.c)
CSRCS_SUBS=$(wildcard $(LIBC_DIR)/*/*.c)

COBJS=$(patsubst $(LIBC_DIR)/%.c, $(LIBC_BUILD_DIR)/c/%.o, $(CSRCS))
COBJS+=$(patsubst $(LIBC_DIR)/%/%.c, $(LIBC_BUILD_DIR)/c/%/%.o, $(CSRCS_SUBS))

ASMSRCS=$(wildcard $(LIBC_DIR)/*.asm)
ASMSRCS_SUBS=$(wildcard $(LIBC_DIR)/*/*.asm)

ASMOBJS=$(patsubst $(LIBC_DIR)/%.asm, $(LIBC_BUILD_DIR)/asm/%.o, $(ASMSRCS))
ASMOBJS+=$(patsubst $(LIBC_DIR)/%/%.asm, $(LIBC_BUILD_DIR)/asm/%/%.o, $(ASMSRCS_SUBS))

STATIC_LIB=$(LIBC_BUILD_DIR)/$(LIBNAME).a

.PHONY: run archive always

run: archive

archive: $(STATIC_LIB)

$(STATIC_LIB): $(COBJS) $(ASMOBJS)
	@printf "$(SHARED_COLORSTART)[LIBC]$(SHARED_COLOREND): Archiving...\n"
	mkdir -p $(@D)
	$(TARGET_AR) qc $@ $^
	$(TARGET_RANLIB) $@

$(LIBC_BUILD_DIR)/c/%.o: $(LIBC_DIR)/%.c
	@printf "$(SHARED_COLORSTART)[LIBC]$(SHARED_COLOREND): Compiling $^\n"
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $^ -o $@ -I$(INC_DIR)

$(LIBC_BUILD_DIR)/asm/%.o: $(LIBC_DIR)/%.asm
	@printf "$(SHARED_COLORSTART)[LIBC]$(SHARED_COLOREND): Assembling $^\n"
	@mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $^ -o $@

always:
	@mkdir -p $(LIBC_BUILD_DIR)