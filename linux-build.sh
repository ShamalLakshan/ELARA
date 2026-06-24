#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"
JOBS=$(nproc 2>/dev/null || echo 4)

echo "[build] Configuring ($BUILD_TYPE)..."

cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "[build] Building with $JOBS jobs..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$JOBS"

echo ""
echo "[build] Done."
echo "  elara_builder   -> $BUILD_DIR/builder/elara_builder"
echo "  elara_validator -> $BUILD_DIR/validator/elara_validator"