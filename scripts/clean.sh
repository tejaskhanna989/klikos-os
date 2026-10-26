#!/usr/bin/env bash
# clean.sh
# Remove all build artifacts
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Cleaning KlikOS build artifacts..."
rm -rf build
echo "==> Done."
