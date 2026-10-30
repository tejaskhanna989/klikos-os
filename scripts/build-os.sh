#!/usr/bin/env bash
# build-os.sh
# Build the full KlikOS OS: toolchain check, kernel, userspace, apps, disk image.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
BUILD_DIR="$ROOT/build"
TARGET="aarch64-klikos-elf"

TOOLCHAIN_PREFIX="$ROOT/toolchain/install"
if [ ! -x "$TOOLCHAIN_PREFIX/bin/${TARGET}-gcc" ]; then
    echo "Error: Cross toolchain not found at $TOOLCHAIN_PREFIX"
    echo "Run ./scripts/build-cross-toolchain.sh first."
    exit 1
fi

export PATH="$TOOLCHAIN_PREFIX/bin:$PATH"

NPROC="${NPROC:-$(nproc)}"

echo "============================================================"
echo "  KlikOS full OS build (from scratch, no Android)"
echo "  Target: $TARGET"
echo "  Jobs:   $NPROC"
echo "============================================================"

mkdir -p "$BUILD_DIR"

# ---------- 1. Kernel ----------
echo ""
echo "=== [1/5] Kernel ==="
make -C "$ROOT" -j"$NPROC" kernel

# ---------- 2. Userspace (libc, init, shell, coreutils) ----------
echo ""
echo "=== [2/5] Userspace ==="
./scripts/build-userspace.sh

# ---------- 3. Apps / framework / display / runtime ----------
echo ""
echo "=== [3/5] Apps & framework ==="
./scripts/build-apps.sh

# ---------- 4. Bootloader stub ----------
echo ""
echo "=== [4/5] Bootloader stub ==="
mkdir -p "$BUILD_DIR/bootloader"
cat > "$BUILD_DIR/bootloader/stub.S" <<'EOF'
.section .text.boot
.global _start
_start:
    wfi
    b       .-4
EOF
"${TARGET}-gcc" -nostdinc -nostdlib -ffreestanding -mcmodel=small \
    -c "$BUILD_DIR/bootloader/stub.S" -o "$BUILD_DIR/bootloader/stub.o"
"${TARGET}-ld" -Ttext=0x40000000 -o "$BUILD_DIR/bootloader/bootloader.elf" "$BUILD_DIR/bootloader/stub.o"
"${TARGET}-objcopy" -O binary "$BUILD_DIR/bootloader/bootloader.elf" "$BUILD_DIR/bootloader/bootloader.bin"

# ---------- 5. Disk image ----------
echo ""
echo "=== [5/5] Disk image ==="
IMG="$BUILD_DIR/klikos.img"
dd if=/dev/zero of="$IMG" bs=1M count=256 status=none

# sudo prompts on the terminal; no password is stored in this script.
run_sudo() {
    sudo "$@"
}

if command -v sudo >/dev/null 2>&1; then
    :
else
    echo "Warning: sudo not found, skipping disk image partitioning."
    exit 1
fi

run_sudo parted -s "$IMG" mklabel gpt
run_sudo parted -s "$IMG" mkpart primary fat32 1MiB 65MiB
run_sudo parted -s "$IMG" set 1 esp on
run_sudo parted -s "$IMG" mkpart primary ext4 65MiB 100%

LOOP=$(run_sudo losetup -fP --show "$IMG")
trap 'run_sudo losetup -d "$LOOP"' EXIT

run_sudo mkfs.vfat -F32 "${LOOP}p1"
run_sudo mkfs.ext4 -F "${LOOP}p2"

run_sudo mkdir -p /tmp/klikos-root
run_sudo mount "${LOOP}p2" /tmp/klikos-root
run_sudo mkdir -p /tmp/klikos-root/boot /tmp/klikos-root/bin /tmp/klikos-root/apps
run_sudo cp "$BUILD_DIR/klikos.bin" /tmp/klikos-root/boot/klikos.bin
run_sudo cp "$BUILD_DIR/klikos.elf" /tmp/klikos-root/boot/klikos.elf
run_sudo cp "$BUILD_DIR/userspace/init.elf" /tmp/klikos-root/init
run_sudo cp "$BUILD_DIR/userspace/shell.elf" /tmp/klikos-root/bin/sh

# Copy system services
run_sudo cp "$BUILD_DIR/apps/display-server.elf" /tmp/klikos-root/bin/display-server
run_sudo cp "$BUILD_DIR/apps/window-manager.elf" /tmp/klikos-root/bin/window-manager
run_sudo cp "$BUILD_DIR/apps/framework.elf" /tmp/klikos-root/bin/framework
run_sudo cp "$BUILD_DIR/apps/runtime.elf" /tmp/klikos-root/bin/runtime

# Copy all 40 system apps
for app in settings files terminal browser appstore whatsapp camera gallery \
           phone messages email calendar clock calculator notes reminders \
           maps weather music podcasts videos tv books news stocks \
           health fitness home shortcuts measure translate voicememos \
           findmy contacts facetime gamecenter testflight tips \
           accessibility systemupdate; do
    if [ -f "$BUILD_DIR/apps/${app}.elf" ]; then
        run_sudo cp "$BUILD_DIR/apps/${app}.elf" /tmp/klikos-root/apps/${app}
    fi
done

run_sudo umount /tmp/klikos-root
run_sudo rmdir /tmp/klikos-root 2>/dev/null || true

echo ""
echo "============================================================"
echo "  KlikOS OS build complete!"
echo "  Kernel ELF:   $BUILD_DIR/klikos.elf"
echo "  Kernel BIN:   $BUILD_DIR/klikos.bin"
echo "  Disk image:   $IMG"
echo ""
echo "  Run: ./scripts/run-qemu.sh"
echo "============================================================"
