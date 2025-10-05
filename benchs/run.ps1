# Windows PowerShell script to run benchmarks with timestamped results and optional comparison
# Usage: .\run.ps1 [--compare] [benchmark_options]

$ErrorActionPreference = "Stop"
Set-Location "$PSScriptRoot/.."

# Parse command line arguments
$ENABLE_COMPARE = $false
$BENCHMARK_ARGS = @()
foreach ($arg in $args) {
    if ($arg -eq "--compare") {
        $ENABLE_COMPARE = $true
    } else {
        $BENCHMARK_ARGS += $arg
    }
}

# Get the directory of this script
$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Definition
$BUILD_DIR = Join-Path $SCRIPT_DIR "..\build"
$RESULTS_DIR = Join-Path $SCRIPT_DIR "results"
$BIN_DIR = Join-Path $SCRIPT_DIR "..\bin\Release"

# Create results directory if it doesn't exist
if (-not (Test-Path $RESULTS_DIR)) {
    New-Item -ItemType Directory -Path $RESULTS_DIR | Out-Null
}

# Generate timestamp
$TIMESTAMP = Get-Date -Format "yyyyMMdd_HHmmss"
$JSON_FILE = Join-Path $RESULTS_DIR "benchmark_${TIMESTAMP}.json"
$TXT_FILE = Join-Path $RESULTS_DIR "benchmark_${TIMESTAMP}.txt"

# Configure for Release, build binary if not exists
$VS_GENERATOR = "Visual Studio 16 2019"
& cmake -S . -B $BUILD_DIR -G "$VS_GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCH=ON -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Detect CPU cores for parallel build
$CORES = (Get-WmiObject -Class Win32_Processor | Measure-Object -Property NumberOfLogicalProcessors -Sum).Sum
if (-not $CORES) { $CORES = 1 }
$THREADS = [Math]::Max($CORES - 1, 1)

& cmake --build $BUILD_DIR --config Release -- /m:$THREADS

# Check if benchmark binary exists
$BENCHMARK_BIN = Join-Path $BIN_DIR "benchs.exe"
if (-not (Test-Path $BENCHMARK_BIN)) {
    Write-Host "Error: Benchmark binary not found at $BENCHMARK_BIN"
    Write-Host "Please build the project first with: cmake --build . --target benchs"
    exit 1
}

Write-Host "Running benchmarks..."
Write-Host "Timestamp: $TIMESTAMP"
Write-Host "Results will be saved to:"
Write-Host "  JSON: $JSON_FILE"
Write-Host "  Text: $TXT_FILE"
Write-Host ""

# Build argument list for benchmark
$benchArgs = @(
    "--benchmark_min_time=0.1s",
    "--benchmark_format=json",
    "--benchmark_out=$JSON_FILE"
) + $BENCHMARK_ARGS

# Run benchmark and save console output to text file
try {
    $output = & $BENCHMARK_BIN $benchArgs 2>&1 | Tee-Object -FilePath $TXT_FILE
    $exitCode = $LASTEXITCODE
    $output | Select-Object -First 10 | ForEach-Object { Write-Host $_ }
    if ($exitCode -eq 0) {
        Write-Host ""
        Write-Host "Benchmark completed successfully!"
        Write-Host "Results saved to:"
        Write-Host "  JSON: $JSON_FILE"
        Write-Host "  Text: $TXT_FILE"

        # Show file sizes for reference
        Write-Host ""
        Write-Host "File sizes:"
        Get-Item $JSON_FILE, $TXT_FILE | ForEach-Object { Write-Host "  $($_.Name): $([Math]::Round($_.Length/1KB,2)) KB" }

        # Run comparison if requested
        if ($ENABLE_COMPARE) {
            Write-Host ""
            Write-Host "Running comparison with previous results..."
            $GENERIC_TOOL = Join-Path $SCRIPT_DIR "..\scripts\benchmark.py"
            if (Test-Path $GENERIC_TOOL) {
                python $GENERIC_TOOL compare `
                    --results-dir=benchs/results `
                    --file-pattern='benchmark_(\d{8}_\d{6})\.json' `
                    --timestamp-format='%Y%m%d_%H%M%S' `
                    --name-prefix='bm_' `
                    --threshold=1.0 `
                    --run-command='./run.ps1'
            } else {
                Write-Host "Warning: Generic benchmark tool not found at $GENERIC_TOOL"
            }
        } else {
            Write-Host ""
            Write-Host "To compare with previous results, run:"
            Write-Host "  .\run.ps1 --compare"
            Write-Host ""
            Write-Host "Or manually compare with:"
            Write-Host "  python ..\scripts\benchmark.py compare \"
            Write-Host "    --results-dir=results \""
            Write-Host "    --file-pattern='benchmark_(\d{8}_\d{6})\.json' \""
            Write-Host "    --timestamp-format='%Y%m%d_%H%M%S' \""
            Write-Host "    --name-prefix='bm_' \""
            Write-Host "    --threshold=1.0 \""
            Write-Host "    --run-command='./run.ps1'\""
        }
    } else {
        Write-Host ""
        Write-Host "Benchmark failed!"
        Write-Host "[DEBUG] benchs.exe exit code: $exitCode"
        Write-Host "[DEBUG] First 10 lines of output:"
        exit 1
    }
}
catch {
    Write-Host ""
    Write-Host "Benchmark failed!"
    Write-Host "[ERROR] $($_.Exception.Message)"
    Write-Host "[ERROR] $($_.Exception)"
    exit 1
}
