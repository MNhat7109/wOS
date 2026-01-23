
include build_scripts/config.mk

DUTIL=$(PYTHON) $(SCRIPT_DIR)/diskutil.py
IMG_PATH=$(BUILD_DIR)/$(OSNAME).img
LOG_PATH=$(BUILD_DIR)/output.log
MNT=/mnt/$(OSNAME)

IMG_SIZE=262144      # in 512-byte sectors
PART1_LABEL=BOOT
PART2_LABEL=ROOT

QEMU_FLAGS=-d int -machine q35 \
	-device piix3-ide,id=ide \
	-device ahci,id=ahci \
	-drive id=disk,file=$(IMG_PATH),format=raw,if=none \
	-device ide-hd,drive=disk,bus=ide.0
QEMU_TEST_FLAGS=-monitor stdio -display none -serial file:$(LOG_PATH)

.PHONY: clean run

run: buildimg
	$(QEMU) $(QEMU_FLAGS)

test: buildimg
	( sleep 5; quit; ) | $(QEMU) $(QEMU_FLAGS) $(QEMU_TEST_FLAGS)

display_log:
	@cat $(LOG_PATH)

# -------------------------
# Build disk image
# -------------------------
$(IMG_PATH): $(BOOT_BUILD_DIR)/stage1/stage1.bin
	@echo "Building disk image..."
	dd if=/dev/zero of=$@ bs=512 count=$(IMG_SIZE)

# Convert to MBR
	$(DUTIL) convert $@ mbr

# Create partitions
	$(DUTIL) create-part $@ --mode primary --start 1MiB --end 64MiB
	$(DUTIL) create-part $@ --mode primary --start 65MiB

# Format partitions
	$(DUTIL) format $@ --part 1 --fs fat32 --reserved-sectors 16 --label $(PART1_LABEL)
	$(DUTIL) format $@ --part 2 --fs fat32 --label $(PART2_LABEL)

# Set boot partition
	$(DUTIL) setboot $@ --part 1 --boot-file $<

# -------------------------
# Mount partition 1 and copy boot files
# -------------------------
mount1: $(IMG_PATH) $(BOOT_BUILD_DIR)/stage2/stage2.bin $(KERNEL_BUILD_DIR)/kernel.elf
	@echo "Mounting partition 1..."
	$(DUTIL) mount $< $(MNT) --part 1
	sudo cp -r $(BASE_DIR)/boot/** $(MNT)/
	sudo cp -r $(word 2, $^) $(MNT)/boot.bin
	sudo cp -r $(word 3, $^) $(MNT)/

unmount1:
	@echo "Unmounting partition 1..."
	$(DUTIL) unmount $(IMG_PATH) $(MNT) --auto-delete

# -------------------------
# Mount partition 2 and copy root files
# -------------------------
mount2: $(IMG_PATH) $(BOOT_BUILD_DIR)/stage2/stage2.bin $(KERNEL_BUILD_DIR)/kernel.elf
	@echo "Mounting partition 2..."
	$(DUTIL) mount $< $(MNT) --part 2
	sudo cp -r $(BASE_DIR)/root/ $(MNT)/
	sudo mkdir -p $(MNT)/boot
	sudo cp -r $(BASE_DIR)/boot/ $(MNT)/boot/
	sudo cp -r $(word 2, $^) $(MNT)/boot/boot.bin
	sudo cp -r $(word 3, $^) $(MNT)/boot/

unmount2:
	@echo "Unmounting partition 2..."
	$(DUTIL) unmount $(IMG_PATH) $(MNT) --auto-delete

buildimg: always $(IMG_PATH) mount1 unmount1 mount2 unmount2
	$(DUTIL) reset

$(BOOT_BUILD_DIR)/stage2/stage2.bin $(BOOT_BUILD_DIR)/stage1/stage1.bin: bootloader

bootloader:
	$(MAKE) -C $(SRC_DIR)/bootloader

$(KERNEL_BUILD_DIR)/kernel.elf: kernel
kernel:
	$(MAKE) -C $(SRC_DIR)/kernel

setup: 
	@echo Setting up directories...
	mkdir -p $(SRC_DIR) $(BUILD_DIR)

always:
	mkdir -p $(BUILD_DIR)

clean: 
	rm -rf $(BUILD_DIR)/**