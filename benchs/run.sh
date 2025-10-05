#!/bin/bash

# Benchmark runner script with timestamped results export and optional comparison
# Usage: ./run_benchmark.sh [--compare] [benchmark_options]

set -e

# Parse command line arguments
ENABLE_COMPARE=false
BENCHMARK_ARGS=()

for arg in "$@"; do
    case $arg in
        --compare)
            ENABLE_COMPARE=true
            shift
            ;;
        *)
            BENCHMARK_ARGS+=("$arg")
            ;;
    esac
done

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESULTS_DIR="$SCRIPT_DIR/results"
BIN_DIR="$SCRIPT_DIR/../bin"

# Create results directory if it doesn't exist
mkdir -p "$RESULTS_DIR"

# Generate timestamp
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
JSON_FILE="$RESULTS_DIR/benchmark_${TIMESTAMP}.json"
TXT_FILE="$RESULTS_DIR/benchmark_${TIMESTAMP}.txt"

# Check if benchmark binary exists
BENCHMARK_BIN="$BIN_DIR/benchs"
if [[ ! -f "$BENCHMARK_BIN" ]]; then
    echo "Error: Benchmark binary not found at $BENCHMARK_BIN"
    echo "Please build the project first with: cmake --build . --target benchs"
    exit 1
fi

echo "Running benchmarks..."
echo "Timestamp: $TIMESTAMP"
echo "Results will be saved to:"
echo "  JSON: $JSON_FILE"
echo "  Text: $TXT_FILE"
echo ""

# Run benchmark with both console output and JSON export
# Console output is also saved to text file for reference
if "$BENCHMARK_BIN" \
    --benchmark_min_time=0.1 \
    --benchmark_format=json \
    --benchmark_out="$JSON_FILE" \
    "${BENCHMARK_ARGS[@]}" | tee "$TXT_FILE"; then
    
    echo ""
    echo "✓ Benchmark completed successfully!"
    echo "Results saved to:"
    echo "  JSON: $JSON_FILE"
    echo "  Text: $TXT_FILE"
    
    # Show file sizes for reference
    if command -v ls >/dev/null 2>&1; then
        echo ""
        echo "File sizes:"
        ls -lh "$JSON_FILE" "$TXT_FILE" | awk '{print "  " $9 ": " $5}'
    fi
    
    # Run comparison if requested
    if [[ "$ENABLE_COMPARE" == "true" ]]; then
        echo ""
        echo "Running comparison with previous results..."
        
        # Check if generic tool exists
        GENERIC_TOOL="$SCRIPT_DIR/../scripts/benchmark.py"
        if [[ -f "$GENERIC_TOOL" ]]; then
            python3 "$GENERIC_TOOL" compare \
                --results-dir=results \
                --file-pattern='benchmark_(\\d{8}_\\d{6})\\.json' \
                --timestamp-format='%Y%m%d_%H%M%S' \

                #!/bin/bash

                # Benchmark runner script with timestamped results and optional comparison
                # Usage: ./run.sh [--compare] [benchmark_options]

                set -e

                # Parse command line arguments
                ENABLE_COMPARE=false
                BENCHMARK_ARGS=()
                for arg in "$@"; do
                    if [[ "$arg" == "--compare" ]]; then
                        ENABLE_COMPARE=true
                    else
                        BENCHMARK_ARGS+=("$arg")
                    fi
                done

                # Get the directory of this script
                SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
                BUILD_DIR="$SCRIPT_DIR/../build"
                RESULTS_DIR="$SCRIPT_DIR/results"
                BIN_DIR="$SCRIPT_DIR/../bin"

                # Create results directory if it doesn't exist
                mkdir -p "$RESULTS_DIR"

                # Generate timestamp
                TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
                JSON_FILE="$RESULTS_DIR/benchmark_${TIMESTAMP}.json"
                TXT_FILE="$RESULTS_DIR/benchmark_${TIMESTAMP}.txt"

                # Configure for Release, build binary if not exists
                VS_GENERATOR="Visual Studio 16 2019"
                cmake -S . -B "$BUILD_DIR" -G "$VS_GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCH=ON -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

                # Detect CPU cores for parallel build
                CORES=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
                THREADS=$((CORES > 1 ? CORES - 1 : 1))

                cmake --build "$BUILD_DIR" --config Release -- -j$THREADS

                # Check if benchmark binary exists
                BENCHMARK_BIN="$BIN_DIR/benchs"
                if [[ ! -f "$BENCHMARK_BIN" ]]; then
                    echo "Error: Benchmark binary not found at $BENCHMARK_BIN"
                    echo "Please build the project first with: cmake --build . --target benchs"
                    exit 1
                fi

                echo "Running benchmarks..."
                echo "Timestamp: $TIMESTAMP"
                echo "Results will be saved to:"
                echo "  JSON: $JSON_FILE"
                echo "  Text: $TXT_FILE"
                echo ""

                # Build argument list for benchmark
                benchArgs=(
                    "--benchmark_min_time=0.1s"
                    "--benchmark_format=json"
                    "--benchmark_out=$JSON_FILE"
                )
                benchArgs+=("${BENCHMARK_ARGS[@]}")

                # Run benchmark and save console output to text file
                {
                    output=$("$BENCHMARK_BIN" "${benchArgs[@]}" 2>&1 | tee "$TXT_FILE")
                    exitCode=${PIPESTATUS[0]}
                    echo "$output" | head -n 10
                    if [[ $exitCode -eq 0 ]]; then
                        echo ""
                        echo "Benchmark completed successfully!"
                        echo "Results saved to:"
                        echo "  JSON: $JSON_FILE"
                        echo "  Text: $TXT_FILE"

                        # Show file sizes for reference
                        echo ""
                        echo "File sizes:"
                        ls -lh "$JSON_FILE" "$TXT_FILE" | awk '{print "  " $9 ": " $5}'

                        # Run comparison if requested
                        if [[ "$ENABLE_COMPARE" == "true" ]]; then
                            echo ""
                            echo "Running comparison with previous results..."
                            GENERIC_TOOL="$SCRIPT_DIR/../scripts/benchmark.py"
                            if [[ -f "$GENERIC_TOOL" ]]; then
                                python3 "$GENERIC_TOOL" compare \
                                    --results-dir=results \
                                    --file-pattern='benchmark_(\\d{8}_\\d{6})\\.json' \
                                    --timestamp-format='%Y%m%d_%H%M%S' \
                                    --name-prefix='bm_' \
                                    --threshold=1.0 \
                                    --run-command='./run.sh'
                            else
                                echo "Warning: Generic benchmark tool not found at $GENERIC_TOOL"
                            fi
                        else
                            echo ""
                            echo "To compare with previous results, run:"
                            echo "  ./run.sh --compare"
                            echo ""
                            echo "Or manually compare with:"
                            echo "  python3 ../scripts/benchmark.py compare \
                    --results-dir=results \
                    --file-pattern='benchmark_(\\d{8}_\\d{6})\\.json' \
                    --timestamp-format='%Y%m%d_%H%M%S' \
                    --name-prefix='bm_' \
                    --threshold=1.0 \
                    --run-command='./run.sh'"
                        fi
                    else
                        echo ""
                        echo "Benchmark failed!"
                        echo "[DEBUG] benchs exit code: $exitCode"
                        echo "[DEBUG] First 10 lines of output:"
                        exit 1
                    fi
                } || {
                    echo ""
                    echo "Benchmark failed!"
                    exit 1
                }