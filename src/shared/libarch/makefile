SHARED_COLORSTART = \e[36m
SHARED_COLOREND = \e[0m

SRC_DIR := $(shell pwd)

ifeq ($(TARGET), i686-elf)
ARCH_DIR = x86/i386
else
$(error Unsupported TARGET: $(TARGET))
endif

.PHONY: all

all:
	@printf "$(SHARED_COLORSTART)[LIBARCH]$(SHARED_COLOREND): Entering $(ARCH_DIR)\n"
	$(MAKE) -C $(SRC_DIR)/$(ARCH_DIR)
