# Windows PowerShell script for Release build: examples and library only
# Usage: .\build.ps1

$ErrorActionPreference = "Stop"

$BUILD_DIR = "build"

# Check VCPKG_ROOT environment variable
if (-not $env:VCPKG_ROOT) {
    Write-Host "Error: VCPKG_ROOT environment variable is not set. Please set it to your vcpkg root directory."
    exit 1
}

# Create build directory if not exists
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

# Configure for Release, build examples and library
$VS_GENERATOR = "Visual Studio 16 2019"
& cmake -S . -B $BUILD_DIR -G "$VS_GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLE=ON -DBUILD_LIB=ON -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Detect CPU cores for parallel build
$CORES = (Get-WmiObject -Class Win32_Processor | Measure-Object -Property NumberOfLogicalProcessors -Sum).Sum
if (-not $CORES) { $CORES = 1 }
$THREADS = [Math]::Max($CORES - 1, 1)

& cmake --build $BUILD_DIR --config Release -- /m:$THREADS