#!/usr/bin/env bash
set -e

BUILD_DIR="build"

# Create build directory if not exists
mkdir -p "$BUILD_DIR"

# Configure for Release, build examples and library
if [ -z "$VCPKG_ROOT" ]; then
  echo "Error: VCPKG_ROOT environment variable is not set. Please set it to your vcpkg root directory."
  exit 1
fi
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLE=ON -DBUILD_LIB=ON -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Detect CPU cores for parallel build
CORES=$(nproc)
if [ "$CORES" -gt 1 ]; then
  THREADS=$((CORES-1))
else
  THREADS=1
fi
cmake --build "$BUILD_DIR" -j "$THREADS"

echo "Build finished. Examples and library are in $BUILD_DIR/bin and $BUILD_DIR/lib."
