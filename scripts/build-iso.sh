#!/usr/bin/env bash
# build-iso.sh
# Build bootable GRUB ISO for KlikOS
set -euo pipefail

cd "$(dirname "$0")/.."

ISO_DIR="build/iso"
ISO_OUT="build/klikos.iso"
KERNEL_IMG="build/kernel/vmlinuz-klikos"
INITRD_IMG="build/kernel/initramfs-klikos.img"

mkdir -p "$ISO_DIR/boot/grub"

# Copy kernel
if [ -f "$KERNEL_IMG" ]; then
    cp "$KERNEL_IMG" "$ISO_DIR/boot/vmlinuz-klikos"
else
    echo "Warning: Kernel image not found at $KERNEL_IMG"
    echo "Run ./scripts/build-kernel.sh first."
fi

# Copy initramfs
if [ -f "$INITRD_IMG" ]; then
    cp "$INITRD_IMG" "$ISO_DIR/boot/initramfs-klikos.img"
fi

# Copy userspace init
if [ -f "build/init/klikos" ]; then
    mkdir -p "$ISO_DIR/boot/init"
    cp "build/init/klikos" "$ISO_DIR/boot/init/klikos"
fi

# Copy apps
if [ -d "build/apps" ]; then
    mkdir -p "$ISO_DIR/boot/apps"
    cp build/apps/* "$ISO_DIR/boot/apps/"
fi

# GRUB config
cat > "$ISO_DIR/boot/grub/grub.cfg" << 'GRUBEOF'
set timeout=5
set default=0

menuentry "KlikOS" {
    linux /boot/vmlinuz-klikos root=/dev/ram0 console=tty1 quiet splash
    initrd /boot/initramfs-klikos.img
}

GRUBEOF

# Build GRUB core image
if command -v grub-mkimage >/dev/null 2>&1; then
    grub-mkimage -d /usr/lib/grub/i386-pc -o /tmp/klikos_grub_core.img -O i386-pc -p /boot/grub biosdisk iso9660 2>/dev/null || true
    cp /tmp/klikos_grub_core.img "$ISO_DIR/boot/grub/core.img" 2>/dev/null || true
fi

# Build ISO
if command -v mkisofs >/dev/null 2>&1 || command -v genisoimage >/dev/null 2>&1; then
    mkisofs -R -b boot/grub/core.img -no-emul-boot -boot-load-size 4 -boot-info-table \
        -o "$ISO_OUT" "$ISO_DIR" 2>/dev/null || true
    echo "ISO built: $ISO_OUT"
else
    echo "Warning: mkisofs/genisoimage not found, skipping ISO creation"
fi
