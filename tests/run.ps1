# Windows PowerShell script to run unit tests and output coverage
# Requires OpenCppCoverage.exe in PATH for Community edition

$ErrorActionPreference = "Stop"
Set-Location "$PSScriptRoot/.."
$BUILD_DIR = "build"
$BIN_DIR = "bin"

# Check VCPKG_ROOT environment variable
if (-not $env:VCPKG_ROOT) {
    Write-Host "Error: VCPKG_ROOT environment variable is not set. Please set it to your vcpkg root directory."
    exit 1
}

# Create build directory if not exists
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

# Configure for Debug, build examples and library
$VS_GENERATOR = "Visual Studio 16 2019"
& cmake -S . -B $BUILD_DIR -G "$VS_GENERATOR" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST=ON -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_CXX_FLAGS="/MP /Zi /Od /RTC1 /DTEST /DDEBUG /DWIN32 /D_WINDOWS /EHsc /MDd /FC /W3 /WX-"

# Detect CPU cores for parallel build
$CORES = (Get-WmiObject -Class Win32_Processor | Measure-Object -Property NumberOfLogicalProcessors -Sum).Sum
if (-not $CORES) { $CORES = 1 }
$THREADS = [Math]::Max($CORES - 1, 1)

& cmake --build $BUILD_DIR --config Debug -- /m:$THREADS

# Check test binary
$TEST_EXE = "$BIN_DIR/Debug/tests.exe"
$SRC_ROOT = (Get-Location).Path
$COVERAGE_WORKDIR = Join-Path $BIN_DIR "Debug"
$COVERAGE_SCRIPT = "$PSScriptRoot/../scripts/coverage.ps1"

if (-not (Test-Path $COVERAGE_SCRIPT)) {
    Write-Host "coverage.ps1 not found: $COVERAGE_SCRIPT"
    exit 1
}

Write-Host "Running coverage.ps1 for coverage analysis..."
& $COVERAGE_SCRIPT -Exe $TEST_EXE -Source $SRC_ROOT -WorkDir $COVERAGE_WORKDIR
