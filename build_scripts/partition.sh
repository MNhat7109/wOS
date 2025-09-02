#!/bin/sh
# Usage: ./populate_partition.sh <partition_image> <stage1> <stage2> <kernel> <root_dir> <mount_point>
set -e

PART_IMG="$1"
STAGE1="$2"
STAGE2="$3"
KERNEL="$4"
ROOT_DIR="$5"
MOUNT_POINT="$6"

echo "[*] Creating empty partition image..."
dd if=/dev/zero of="$PART_IMG" bs=512 count=67584

echo "[*] Formatting FAT32..."
mkfs.fat -F 32 -R 32 "$PART_IMG"

echo "[*] Writing stage1..."
dd if="$STAGE1" of="$PART_IMG" bs=512 count=1 conv=notrunc

echo "[*] Writing stage2..."
dd if="$STAGE2" of="$PART_IMG" bs=512 count=2 seek=2 skip=1 conv=notrunc

echo "[*] Writing kernel..."
dd if="$KERNEL" of="$PART_IMG" bs=512 seek=6 conv=notrunc

echo "[*] Preparing root files..."
mkdir -p "$ROOT_DIR/part1"
cp "$STAGE2" "$ROOT_DIR/part1/boot.bin"
cp "$KERNEL" "$ROOT_DIR/part1/"

echo "[*] Mounting partition..."
sudo mkdir -p "$MOUNT_POINT"
sudo mount -o loop "$PART_IMG" "$MOUNT_POINT"

echo "[*] Copying files to partition..."
sudo cp -r "$ROOT_DIR/part1/"* "$MOUNT_POINT/"

echo "[*] Unmounting..."
sudo umount "$MOUNT_POINT"
sudo rm -rf "$MOUNT_POINT"

echo "[*] Partition populated successfully!"
