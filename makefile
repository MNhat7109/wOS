
include build_scripts/config.mk

.PHONY: clean run

run: buildimg
	qemu-system-i386 \
	-machine q35 \
	-device piix3-ide,id=ide \
	-drive id=disk,file=$(BUILD_DIR)/$(OSNAME).img,format=raw,if=none \
	-device ide-hd,drive=disk,bus=ide.0

buildimg: always $(BUILD_DIR)/$(OSNAME).img

$(BUILD_DIR)/$(OSNAME).img: $(BOOT_DIR)/$(OSNAME)_p1.img $(BOOT_DIR)/mbr/mbr.bin
	@echo Building image...
	dd if=/dev/zero of=$@ bs=512 count=69632
	dd if=$(word 2, $^) of=$@ conv=notrunc
	dd if=$< of=$@ bs=512 seek=2048 conv=notrunc

$(BOOT_DIR)/$(OSNAME)_p1.img: $(BOOT_DIR)/stage1/stage1.bin $(BOOT_DIR)/stage2/stage2.bin $(KRNL_DIR)/kernel.elf
	dd if=/dev/zero of=$@ bs=512 count=67584
	mkfs.fat -F 32 -R 32 $@
	dd if=$< of=$@ bs=512 count=1 conv=notrunc
	dd if=$< of=$@ bs=512 count=2 seek=2 skip=1 conv=notrunc
	dd if=$< of=$@ bs=512 seek=6 conv=notrunc
	cp $(word 2, $^) $(SRC_DIR)/files/part1/boot.bin
	cp $(word 3, $^) $(SRC_DIR)/files/part1/
	sudo mkdir -p /mnt/$(OSNAME)
	sudo mount -t vfat -o loop $@ /mnt/$(OSNAME)
	sudo cp -r $(SRC_DIR)/files/part1/** /mnt/$(OSNAME)
	sudo umount /mnt/$(OSNAME)
	sudo rm -rf /mnt/$(OSNAME)/**
	

$(BOOT_DIR)/stage2/stage2.bin $(BOOT_DIR)/stage1/stage1.bin $(BOOT_DIR)/mbr/mbr.bin: bootloader

bootloader:
	$(MAKE) -C $(SRC_DIR)/bootloader

$(KRNL_DIR)/kernel.elf: kernel
kernel:
	$(MAKE) -C $(SRC_DIR)/kernel

setup: 
	@echo Setting up directories...
	mkdir -p $(SRC_DIR) $(BUILD_DIR)

always:
	mkdir -p $(BUILD_DIR)

clean: 
	rm -rf $(BUILD_DIR)/**