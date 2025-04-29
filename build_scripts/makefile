
DIR=$(abspath ../toolchains)

TARGET=i686-elf
PREFIX=$(DIR)/$(TARGET)

.PHONY: run
run: $(PREFIX)/bin/$(TARGET)-ld $(PREFIX)/bin/$(TARGET)-gcc
	@echo "export PATH=$(PREFIX)/bin:$$PATH" >> ~/.bashrc
	@source ~/.bashrc

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