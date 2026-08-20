LIBDS_INC_DIR=$(INC_DIR)/ds
LIBDS_INTERNAL_INC_DIR=$(LIBDS_INC_DIR)/internals

LIBNAME=libds

CSRCS=$(wildcard $(LIBDS_DIR)/*.c)
CSRCS_SUBS=$(wildcard $(LIBDS_DIR)/*/*.c)

COBJS=$(patsubst $(LIBDS_DIR)/%.c, $(LIBDS_BUILD_DIR)/c/%.o, $(CSRCS))
COBJS+=$(patsubst $(LIBDS_DIR)/%.c, $(LIBDS_BUILD_DIR)/c/%.o, $(CSRCS_SUBS))

STATIC_LIB=$(LIBDS_BUILD_DIR)/$(LIBNAME).a

.PHONY: run archive always

run: archive

archive: $(STATIC_LIB)

$(STATIC_LIB): $(COBJS)
	@printf "[LIBDS]: Archiving...\n"
	mkdir -p $(@D)
	$(TARGET_AR) qc $@ $^
	$(TARGET_RANLIB) $@

$(LIBDS_BUILD_DIR)/c/%.o: $(LIBDS_DIR)/%.c
	@printf "[LIBDS]: Compiling $^\n"
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -g -c $^ -o $@ -I$(LIBDS_INC_DIR) -I$(LIBDS_INTERNAL_INC_DIR)

always:
	@mkdir -p $(LIBDS_BUILD_DIR)