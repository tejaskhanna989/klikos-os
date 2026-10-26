#!/usr/bin/env bash
# run-qemu.sh — Build and boot KlikOS in QEMU with graphical GTK window
set -euo pipefail

cd "$(dirname "$0")/.."

# Ensure image exists
if [ ! -f build/kernel/vmlinuz-klikos ] || [ ! -f build/kernel/initramfs-klikos.img ]; then
    echo "[klikos] Image not found, building..."
    ./scripts/build-all.sh
fi

echo "[klikos] Starting QEMU x86_64 (GTK window, Ctrl+Alt+G to release mouse)..."
qemu-system-x86_64 \
    -machine q35 \
    -cpu max \
    -smp 4 \
    -m 2048M \
    -serial stdio \
    -vga std \
    -device ramfb \
    -display gtk,grab-on-hover=on,full-screen=off \
    -no-reboot \
    -kernel build/kernel/vmlinuz-klikos \
    -initrd build/kernel/initramfs-klikos.img \
    -append "console=ttyS0 init=/init quiet" \
    "$@"
