include config.mk

DIR=$(abspath ../toolchains)

TARGET+=-elf # Previously, TARGET holds only the arch (e.g. i686, x86_64, etc.). Add "-elf" so that everything won't break
PREFIX=$(DIR)/$(TARGET)

.PHONY: run
run: $(PREFIX)/bin/$(TARGET)-ld $(PREFIX)/bin/$(TARGET)-gcc

$(PREFIX)/bin/$(TARGET)-ld: binutils

binutils: always always-binutils
	./binutils_latest.sh $(DIR) $(DIR)/binutils_build $(TARGET) $(PREFIX)

$(PREFIX)/bin/$(TARGET)-gcc: gcc

gcc: always always-gcc
	./gcc_latest.sh $(DIR) $(DIR)/gcc_build $(TARGET) $(PREFIX)

always:
	mkdir -p $(DIR)
	mkdir -p $(PREFIX)

always-binutils:
	mkdir -p $(DIR)/binutils_build

always-gcc:
	mkdir -p $(DIR)/gcc_build

clean:
	rm -rf $(DIR)