#!/usr/bin/env bash
# build-cross-toolchain.sh
# Build a bare-metal aarch64-klikos-elf cross toolchain (binutils + GCC).
# Installs into toolchain/install/ under the project root.
#
# This takes 20–40 minutes depending on CPU and network.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
TOOLCHAIN_DIR="$ROOT/toolchain"
SRC_DIR="$TOOLCHAIN_DIR/src"
INSTALL_DIR="$TOOLCHAIN_DIR/install"
TARGET="aarch64-klikos-elf"

# Versions (pinned for reproducibility)
BINUTILS_VERSION="2.42"
GCC_VERSION="13.2.0"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"

NPROC="${NPROC:-$(nproc)}"

mkdir -p "$SRC_DIR" "$INSTALL_DIR"

export PATH="$INSTALL_DIR/bin:$PATH"
export CFLAGS="-O2"
export CXXFLAGS="-O2"

echo "==> Target:   $TARGET"
echo "==> Install:  $INSTALL_DIR"
echo "==> Jobs:     $NPROC"

# ---------- Download ----------
download() {
    local url="$1" file="$2"
    if [ -f "$file" ]; then
        echo "==> Already downloaded: $file"
        return
    fi
    echo "==> Downloading: $url"
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 -o "$file" "$url"
    else
        wget -q -O "$file" "$url"
    fi
}

download "$BINUTILS_URL" "$SRC_DIR/binutils.tar.xz"
download "$GCC_URL"      "$SRC_DIR/gcc.tar.xz"

# ---------- Extract ----------
extract() {
    local file="$1" dir="$2"
    if [ -d "$dir" ]; then
        echo "==> Already extracted: $dir"
        return
    fi
    echo "==> Extracting: $file"
    tar -xJf "$file" -C "$SRC_DIR"
}

extract "$SRC_DIR/binutils.tar.xz" "$SRC_DIR/binutils-$BINUTILS_VERSION"
extract "$SRC_DIR/gcc.tar.xz"      "$SRC_DIR/gcc-$GCC_VERSION"

# ---------- Build binutils ----------
BINUTILS_SRC="$SRC_DIR/binutils-$BINUTILS_VERSION"
BINUTILS_BUILD="$TOOLCHAIN_DIR/build-binutils"

echo "==> Configuring binutils..."
mkdir -p "$BINUTILS_BUILD"
(
    cd "$BINUTILS_BUILD"
    "$BINUTILS_SRC/configure" \
        --target="$TARGET" \
        --prefix="$INSTALL_DIR" \
        --with-sysroot \
        --disable-nls \
        --disable-werror
)
echo "==> Building binutils..."
make -C "$BINUTILS_BUILD" -j"$NPROC"
make -C "$BINUTILS_BUILD" install

# ---------- Build GCC (bootstrap) ----------
GCC_SRC="$SRC_DIR/gcc-$GCC_VERSION"
GCC_BUILD="$TOOLCHAIN_DIR/build-gcc"

# GCC needs GMP, MPFR, MPC. Try to fetch them automatically via contrib script.
if [ -x "$GCC_SRC/contrib/download_prerequisites" ]; then
    echo "==> Downloading GCC prerequisites (gmp, mpfr, mpc)..."
    (
        cd "$GCC_SRC"
        ./contrib/download_prerequisites
    )
fi

echo "==> Configuring GCC..."
mkdir -p "$GCC_BUILD"
(
    cd "$GCC_BUILD"
    "$GCC_SRC/configure" \
        --target="$TARGET" \
        --prefix="$INSTALL_DIR" \
        --without-headers \
        --with-newlib \
        --disable-nls \
        --disable-shared \
        --disable-threads \
        --disable-libssp \
        --disable-libstdcxx-pch \
        --disable-libatomic \
        --disable-libgomp \
        --disable-libquadmath \
        --disable-libvtv \
        --disable-libcilkrts \
        --disable-libsanitizer \
        --disable-libmpx \
        --enable-languages=c \
        --without-gnu-as \
        --without-gnu-ld \
        --disable-werror
)
echo "==> Building GCC (all-gcc + target-libgcc)..."
make -C "$GCC_BUILD" -j"$NPROC" all-gcc
make -C "$GCC_BUILD" -j"$NPROC" all-target-libgcc
make -C "$GCC_BUILD" install-gcc
make -C "$GCC_BUILD" install-target-libgcc

echo ""
echo "==> Toolchain built successfully at: $INSTALL_DIR"
echo "==> Add to PATH:"
echo "    export PATH=\"$INSTALL_DIR/bin:\$PATH\""
echo "==> Verify with:"
echo "    $TARGET-gcc --version"
