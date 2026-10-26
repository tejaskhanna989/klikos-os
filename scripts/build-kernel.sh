#!/usr/bin/env bash
# build-kernel.sh
# Download and build Linux kernel for KlikOS
set -euo pipefail

cd "$(dirname "$0")/.."

KERNEL_SRC="src/kernel/linux"
KERNEL_VERSION="6.6.56"
KERNEL_TAR="linux-${KERNEL_VERSION}.tar.xz"
KERNEL_URL="https://cdn.kernel.org/pub/linux/kernel/v6.x/${KERNEL_TAR}"
CONFIG_FILE="config/kernel_defconfig"
OUTPUT_DIR="build/kernel"

mkdir -p "$OUTPUT_DIR"

# Download kernel if not present
if [ ! -d "$KERNEL_SRC" ]; then
    echo "Downloading Linux kernel v${KERNEL_VERSION}..."
    mkdir -p src/kernel
    cd src/kernel
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 -o "$KERNEL_TAR" "$KERNEL_URL"
    else
        wget -q -O "$KERNEL_TAR" "$KERNEL_URL"
    fi
    tar -xJf "$KERNEL_TAR"
    mv "linux-${KERNEL_VERSION}" linux
    cd ../..
fi

# Copy config
echo "Applying KlikOS kernel configuration..."
cp "$CONFIG_FILE" "$KERNEL_SRC/.config"

# Build kernel
echo "Building Linux kernel (this may take 20-40 minutes)..."
make -C "$KERNEL_SRC" -j"$(nproc)" olddefconfig >/dev/null 2>&1 || true
make -C "$KERNEL_SRC" -j"$(nproc)" \
    INSTALL_MOD_PATH="$OUTPUT_DIR/modules" \
    INSTALL_MOD_STRIP=1 \
    modules 2>&1 | tail -5

make -C "$KERNEL_SRC" -j"$(nproc)" \
    INSTALL_MOD_PATH="$OUTPUT_DIR/modules" \
    modules_install 2>&1 | tail -5

# Copy kernel image
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
    cp "$KERNEL_SRC/arch/x86/boot/bzImage" "$OUTPUT_DIR/vmlinuz-klikos"
    echo "Kernel image: $OUTPUT_DIR/vmlinuz-klikos"
else
    cp "$KERNEL_SRC/arch/arm64/boot/Image" "$OUTPUT_DIR/vmlinuz-klikos"
    echo "Kernel image: $OUTPUT_DIR/vmlinuz-klikos"
fi

echo "Kernel build complete!"
