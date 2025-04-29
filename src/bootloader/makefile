BOOT_COLORSTART = \e[34m
BOOT_COLOREND = \e[0m

.PHONY: mbr stage1 stage2

stage2: stage1 mbr
	@printf "$(BOOT_COLORSTART)[BOOTLOADER]$(BOOT_COLOREND): Entering $@\n" 
	@$(MAKE) -C stage2/

stage1: mbr
	@printf "$(BOOT_COLORSTART)[BOOTLOADER]$(BOOT_COLOREND): Entering $@\n" 
	@$(MAKE) -C stage1/

mbr:
	@printf "$(BOOT_COLORSTART)[BOOTLOADER]$(BOOT_COLOREND): Entering $@\n" 
	@$(MAKE) -C mbr/
