#!/usr/bin/env bash
set -e

BUILD_DIR="../build"
BIN_DIR="../bin"

# Build with coverage flags
echo "Building with coverage..."
cmake -S .. -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST=ON -DCMAKE_CXX_FLAGS="--coverage"
CORES=$(nproc)
if [ "$CORES" -gt 1 ]; then
	THREADS=$((CORES-1))
else
	THREADS=1
fi
cmake --build "$BUILD_DIR" -j "$THREADS"

# Run tests
echo "Running tests..."
cd "$BIN_DIR"
./tests
cd -

# Generate coverage report
echo "Generating coverage report..."
lcov --capture --directory "$BUILD_DIR" --output-file "$BUILD_DIR/coverage.info"
lcov --list "$BUILD_DIR/coverage.info"

# Optional: generate HTML report
genhtml "$BUILD_DIR/coverage.info" --output-directory "$BUILD_DIR/coverage_html"
echo "Coverage HTML report generated in $BUILD_DIR/coverage_html/"
