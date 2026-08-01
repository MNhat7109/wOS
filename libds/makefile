SHARED_COLORSTART = \e[36m
SHARED_COLOREND = \e[0m

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
	@printf "$(SHARED_COLORSTART)[LIBDS]$(SHARED_COLOREND): Archiving...\n"
	mkdir -p $(@D)
	$(TARGET_AR) qc $@ $^
	$(TARGET_RANLIB) $@

$(LIBDS_BUILD_DIR)/c/%.o: $(LIBDS_DIR)/%.c
	@printf "$(SHARED_COLORSTART)[LIBDS]$(SHARED_COLOREND): Compiling $^\n"
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $^ -o $@ -I$(INC_DIR)/ds

always:
	@mkdir -p $(LIBDS_BUILD_DIR)