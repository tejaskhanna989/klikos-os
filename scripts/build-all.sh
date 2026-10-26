#!/usr/bin/env bash
# build-all.sh — Build complete KlikOS and create boot image
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
KERNEL_SRC="src/kernel/linux"
KERNEL_OUT="build/kernel"

echo "=== 1/5 Compiling userspace (kernel-independent parts) ==="
make clean >/dev/null 2>&1
make userspace apps >/dev/null 2>&1 || make userspace apps
echo "Userspace built: ls -R build/init build/apps | tail -5"

echo ""
echo "=== 2/5 Building kernel (download/build once) ==="
if [ ! -f "$KERNEL_OUT/bzImage" ]; then
    if [ -d "$KERNEL_SRC" ]; then
        echo "Kernel source present, configuring and building..."
        cp config/kernel_defconfig "$KERNEL_SRC/.config"
        make -C "$KERNEL_SRC" -j"$(nproc)" bzImage 2>&1 | tail -3
        cp "$KERNEL_SRC/arch/x86/boot/bzImage" "$KERNEL_OUT/" 2>/dev/null || \
            cp "$KERNEL_SRC/arch/x86_64/boot/bzImage" "$KERNEL_OUT/" 2>/dev/null || true
    else
        echo "No kernel source. Using host kernel for QEMU boot."
    fi
fi

echo ""
echo "=== 3/5 Assembling initramfs ==="
mkdir -p build/initramfs_final
# Use static init (rebuild if missing)
if [ ! -f /tmp/klikos_init2 ] || [ ! -x /tmp/klikos_init2 ]; then
    mkdir -p /tmp/klikos_init2
    gcc -static -nostdlib -nostartfiles -Ttext 0x400000 -o /tmp/klikos_init2 \
        src/userspace/init/entry.c 2>/dev/null || \
    gcc -static -o /tmp/klikos_init2 - <<'EOF'
#include <unistd.h>
extern int klikos_main(int argc, char **argv);
void _start(void) {
    klikos_main(0, 0);
    for (;;) { _exit(1); }
}
EOF
fi
cp /tmp/klikos_init2 build/initramfs_final/init
chmod +x build/initramfs_final/init
mkdir -p build/initramfs_final/bin build/initramfs_final/lib \
         build/initramfs_final/lib64 build/initramfs_final/sys \
         build/initramfs_final/etc build/initramfs_final/dev \
         build/initramfs_final/proc
for f in build/apps/*; do
    [ -f "$f" ] && cp "$f" build/initramfs_final/bin/
done
cp build/init/klikos build/initramfs_final/bin/ 2>/dev/null || true

if [ -f /usr/lib/libc.so.6 ]; then
    cp /usr/lib/libc.so.6  build/initramfs_final/lib/
    cp /usr/lib/libm.so.6  build/initramfs_final/lib/
    cp /lib64/ld-linux-x86-64.so.2 build/initramfs_final/lib64/
fi

cd build/initramfs_final
find . | cpio -H newc -o 2>/dev/null | gzip > ../initramfs-klikos.img
cd ../..

mkdir -p build/kernel
if [ -f build/kernel/bzImage ]; then
    cp build/kernel/bzImage build/kernel/bzImage.klikos
else
    cp /usr/lib/modules/$(uname -r)/vmlinuz build/kernel/vmlinuz-klikos
fi
cp build/initramfs-klikos.img build/kernel/

echo ""
echo "=== 4/5 Creating GRUB boot image ==="
mkdir -p build/iso/boot/grub
cp build/kernel/vmlinuz-klikos build/iso/boot/vmlinuz-klikos 2>/dev/null || true
cat > build/iso/boot/grub/grub.cfg <<'EOF'
set timeout=5
set default=0
menuentry "KlikOS" {
    insmod all_video
    insmod gzio
    we don't have to exist
    insmod part_gpt
    insmod ext2
    linux /boot/vmlinuz-klikos root=/dev/ram0 console=tty1 quiet 2>&1 | grep -v "We don't have"
    initrd /boot/initramfs-klikos.img
}
EOF
echo "ISO stub created (see run-qemu.sh for boot options)"

echo ""
echo "=== 5/5 Done ==="
ls -la build/klikos.elf build/init/klikos build/apps/ | head -5
echo ""
echo "Run with:  ./scripts/run-qemu.sh"
