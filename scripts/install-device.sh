#!/usr/bin/env bash
# install-device.sh
# Install KlikOS to a target block device using sudo
set -euo pipefail

cd "$(dirname "$0")/.."

SUDO_PASS="t"
run_sudo() {
    echo "$SUDO_PASS" | sudo -S "$@"
}

DEVICE="${1:-}"

if [ -z "$DEVICE" ]; then
    echo "Usage: $0 <block-device>"
    echo "Example: $0 /dev/sdb"
    echo ""
    echo "WARNING: This will WIPE the target device."
    exit 1
fi

if [ ! -b "$DEVICE" ]; then
    echo "Error: $DEVICE is not a block device."
    exit 1
fi

echo "========================================================"
echo "  KlikOS device installer"
echo "  Target: $DEVICE"
echo "  WARNING: ALL DATA ON $DEVICE WILL BE ERASED"
echo "========================================================"
echo ""
read -rp "Type 'yes' to continue: " confirm
if [ "$confirm" != "yes" ]; then
    echo "Aborted."
    exit 1
fi

echo "==> Checking sudo is available..."
if ! command -v sudo >/dev/null 2>&1; then
    echo "Error: sudo not found."
    exit 1
fi

# Test sudo access
echo "$SUDO_PASS" | sudo -S -v 2>/dev/null || {
    echo "Error: sudo authentication failed."
    exit 1
}

KERNEL_IMG="build/kernel/vmlinuz-klikos"
if [ ! -f "$KERNEL_IMG" ]; then
    echo "Error: Kernel image not found at $KERNEL_IMG"
    echo "Run ./scripts/build-kernel.sh first."
    exit 1
fi

echo "==> Unmounting any mounted partitions on $DEVICE..."
for part in $(lsblk -n -o NAME "$DEVICE" | tail -n +2); do
    PKPART="/dev/$part"
    if mountpoint -q "$PKPART" 2>/dev/null || grep -qs "$PKPART" /proc/mounts; then
        run_sudo umount "$PKPART" 2>/dev/null || true
    fi
done

echo "==> Writing partition table (GPT)..."
run_sudo sgdisk --zap-all "$DEVICE"
run_sudo sgdisk --new=1:2048:67583 --typecode=1:ef00 "$DEVICE"   # EFI system partition
run_sudo sgdisk --new=2:67584:last   --typecode=2:8300 "$DEVICE"   # rootfs

echo "==> Creating filesystems..."
run_sudo mkfs.vfat -F32 "${DEVICE}1"
run_sudo mkfs.ext4 -F "${DEVICE}2"

echo "==> Installing kernel and initramfs..."
run_sudo mkdir -p /tmp/klikos-boot
run_sudo mount "${DEVICE}1" /tmp/klikos-boot
run_sudo mkdir -p /tmp/klikos-boot/EFI/klikos
run_sudo cp "build/kernel/vmlinuz-klikos" /tmp/klikos-boot/EFI/klikos/
if [ -f "build/kernel/initramfs-klikos.img" ]; then
    run_sudo cp "build/kernel/initramfs-klikos.img" /tmp/klikos-boot/EFI/klikos/
fi
run_sudo umount /tmp/klikos-boot
run_sudo rmdir /tmp/klikos-boot 2>/dev/null || true

echo ""
echo "==> KlikOS installed to $DEVICE"
echo "    You can now boot from this device."
echo "    For testing, use: ./scripts/run-qemu.sh"
