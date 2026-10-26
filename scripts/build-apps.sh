#!/usr/bin/env bash
# build-apps.sh
# Build KlikOS apps, framework, display server, runtime.
# Builds all 40 system apps as separate aarch64-klikos-elf ELF binaries.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
BUILD_DIR="$ROOT/build/apps"
TARGET="aarch64-klikos-elf"

TOOLCHAIN_PREFIX="$ROOT/toolchain/install"
if [ ! -x "$TOOLCHAIN_PREFIX/bin/${TARGET}-gcc" ]; then
    echo "Error: Cross toolchain not found. Run ./scripts/build-cross-toolchain.sh first."
    exit 1
fi

export PATH="$TOOLCHAIN_PREFIX/bin:$PATH"
export CC="${TARGET}-gcc"
export LD="${TARGET}-ld"

NPROC="${NPROC:-$(nproc)}"
LIBC="$ROOT/build/userspace/libc/libc.a"
LIBC_INC="$ROOT/src/userspace/libc/include"

mkdir -p "$BUILD_DIR"

# App list: name -> source file -> load address
# All 40 system apps
declare -a APPS=(
    "settings|src/apps/settings/main.c|0x80140000"
    "files|src/apps/files/main.c|0x80150000"
    "terminal|src/apps/terminal/main.c|0x80160000"
    "browser|src/apps/browser/main.c|0x80170000"
    "appstore|src/apps/appstore/main.c|0x80180000"
    "whatsapp|src/apps/whatsapp/main.c|0x80190000"
    "camera|src/apps/camera/main.c|0x801a0000"
    "gallery|src/apps/gallery/main.c|0x801b0000"
    "phone|src/apps/phone/main.c|0x801c0000"
    "messages|src/apps/messages/main.c|0x801d0000"
    "email|src/apps/email/main.c|0x801e0000"
    "calendar|src/apps/calendar/main.c|0x801f0000"
    "clock|src/apps/clock/main.c|0x80200000"
    "calculator|src/apps/calculator/main.c|0x80210000"
    "notes|src/apps/notes/main.c|0x80220000"
    "reminders|src/apps/reminders/main.c|0x80230000"
    "maps|src/apps/maps/main.c|0x80240000"
    "weather|src/apps/weather/main.c|0x80250000"
    "music|src/apps/music/main.c|0x80260000"
    "podcasts|src/apps/podcasts/main.c|0x80270000"
    "videos|src/apps/videos/main.c|0x80280000"
    "tv|src/apps/tv/main.c|0x80290000"
    "books|src/apps/books/main.c|0x802a0000"
    "news|src/apps/news/main.c|0x802b0000"
    "stocks|src/apps/stocks/main.c|0x802c0000"
    "health|src/apps/health/main.c|0x802d0000"
    "fitness|src/apps/fitness/main.c|0x802e0000"
    "home|src/apps/home/main.c|0x802f0000"
    "shortcuts|src/apps/shortcuts/main.c|0x80300000"
    "measure|src/apps/measure/main.c|0x80310000"
    "translate|src/apps/translate/main.c|0x80320000"
    "voicememos|src/apps/voicememos/main.c|0x80330000"
    "findmy|src/apps/findmy/main.c|0x80340000"
    "contacts|src/apps/contacts/main.c|0x80350000"
    "facetime|src/apps/facetime/main.c|0x80360000"
    "gamecenter|src/apps/gamecenter/main.c|0x80370000"
    "testflight|src/apps/testflight/main.c|0x80380000"
    "tips|src/apps/tips/main.c|0x80390000"
    "accessibility|src/apps/accessibility/main.c|0x803a0000"
    "systemupdate|src/apps/systemupdate/main.c|0x803b0000"
)

# System services
declare -a SERVICES=(
    "display-server|src/display/server/compositor.c|0x80100000"
    "window-manager|src/display/wm/wm.c|0x80110000"
    "framework|src/framework/framework.c|0x80120000"
    "runtime|src/runtime/runtime.c|0x80130000"
)

build_app() {
    local name="$1" src="$2" text_addr="$3"
    echo "==> Building $name..."
    "${CC}" -c -ffreestanding -I "$LIBC_INC" \
        "$src" -o "$BUILD_DIR/${name}.o"
    "${LD}" -static -nostdlib \
        -Ttext="$text_addr" \
        -o "$BUILD_DIR/${name}.elf" \
        "$BUILD_DIR/${name}.o" "$LIBC"
}

echo "==> Building system services..."
for entry in "${SERVICES[@]}"; do
    IFS='|' read -r name src addr <<< "$entry"
    build_app "$name" "$src" "$addr"
done

echo ""
echo "==> Building 40 system apps..."
for entry in "${APPS[@]}"; do
    IFS='|' read -r name src addr <<< "$entry"
    build_app "$name" "$src" "$addr"
done

echo ""
echo "==> Apps/framework built:"
ls -la "$BUILD_DIR"/*.elf | wc -l
echo "  total ELF binaries"
