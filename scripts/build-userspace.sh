#!/usr/bin/env bash
# build-userspace.sh
# Build KlikOS userspace: display server, toolkit, shell, init
set -euo pipefail

cd "$(dirname "$0")/.."

echo "Building KlikOS userspace..."

# Compile all source files
make -f Makefile userspace 2>&1

echo "Userspace build complete!"
echo "Init binary: build/init/init"
echo "Display lib: build/display/display.o"
echo "Toolkit lib: build/toolkit/toolkit.o"
echo "Shell lib:   build/shell/shell.o"
