include config.mk

DIR=$(abspath ../toolchains)

TC_TARGET=$(TARGET)-elf # Previously, TARGET holds only the arch (e.g. i686, x86_64, etc.). Add "-elf" so that everything won't break
PREFIX=$(DIR)/$(TC_TARGET)

.PHONY: run
run: $(PREFIX)/bin/$(TC_TARGET)-ld $(PREFIX)/bin/$(TC_TARGET)-gcc

$(PREFIX)/bin/$(TC_TARGET)-ld: binutils

binutils: always always-binutils
	./binutils_latest.sh $(DIR) $(DIR)/binutils_build $(TC_TARGET) $(PREFIX)

$(PREFIX)/bin/$(TC_TARGET)-gcc: gcc

gcc: always always-gcc
	./gcc_latest.sh $(DIR) $(DIR)/gcc_build $(TC_TARGET) $(PREFIX)

always:
	mkdir -p $(DIR)
	mkdir -p $(PREFIX)

always-binutils:
	mkdir -p $(DIR)/binutils_build

always-gcc:
	mkdir -p $(DIR)/gcc_build

clean:
	rm -rf $(DIR)