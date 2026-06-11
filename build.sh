#!/usr/bin/env bash
set -euo pipefail

# Simple Build
# cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/USER/Coding/vcpkg/scripts/buildsystems/vcpkg.cmake

# Config
BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"   # pass Debug as first arg to override
VCPKG_ROOT="${VCPKG_ROOT:-C:/Users/USER/Coding/vcpkg}"
TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
JOBS=$(nproc 2>/dev/null || echo 4)

# Checks 
if [ ! -f "$TOOLCHAIN" ]; then
    echo "[build] ERROR: vcpkg toolchain not found at $TOOLCHAIN"
    echo "        Set VCPKG_ROOT env var or edit this script."
    exit 1
fi

# Configure
echo "[build] configuring ($BUILD_TYPE)..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
echo "[build] building with $JOBS jobs..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$JOBS"

# Report 
echo ""
echo "[build] done."
echo "  elara_builder   -> $BUILD_DIR/builder/elara_builder.exe"
echo "  elara_validator -> $BUILD_DIR/validator/elara_validator.exe"