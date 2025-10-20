# Installation & Build (Linux / macOS / Windows)

This document covers getting a working development environment for building and testing the `high-jump` project on Linux, macOS and Windows. It focuses on using CMake + vcpkg (recommended) and Visual Studio on Windows.

## Prerequisites

- CMake 3.19+ (3.21+ recommended)
- A modern C++17-capable compiler:
  - Linux: GCC 9+/11+ or Clang 9+/12+
  - macOS: Xcode command line tools (Clang)
  - Windows: Visual Studio 2019 (MSVC)
- vcpkg (recommended) for dependency management
- Ninja (optional) for fast builds or use the Visual Studio generator

## Prepare vcpkg

1. Clone vcpkg and bootstrap it (if you don't already have vcpkg):

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
```

On Windows (PowerShell):

```powershell
git clone https://github.com/microsoft/vcpkg.git %USERPROFILE%\\vcpkg
cd %USERPROFILE%\\vcpkg
.\bootstrap-vcpkg.bat
```

2. Integrate or reference the vcpkg toolchain in CMake. You can either integrate globally or pass the toolchain file per-project:

```bash
# per-project CMake invocation
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

## Linux / macOS: build & test (recommended flow)

```bash
# from project root
export VCPKG_ROOT=~/vcpkg
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build -- -j$(nproc)
ctest --test-dir build --output-on-failure
```

Notes:
- If you encounter missing packages, run `vcpkg install` (see `vcpkg.json`) or use `vcpkg install --manifest` from the project root.
- If you need AddressSanitizer, set `-DASAN=ON` for supported compilers.

## Windows (Visual Studio 2019)

Open an 'x64 Native Tools Command Prompt for VS 2019' (or run `VsDevCmd.bat`) and then:

```powershell
# from project root
$env:VCPKG_ROOT = 'C:\path\to\vcpkg'
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\\scripts\\buildsystems\\vcpkg.cmake" -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build --config Debug
# run tests
ctest --test-dir build --output-on-failure -C Debug
```

Important Windows notes:
- Run CMake from a Visual Studio developer prompt (via `VsDevCmd.bat`) so the Windows SDK and MSVC environment variables (LIB, INCLUDE) are correctly set; otherwise linker errors such as `kernel32.lib` missing may occur.
- If using the Visual Studio generator (`-G "Visual Studio 16 2019"`), open the generated solution in Visual Studio and build using the IDE.

## Running benchmarks

Benchmarks are located in `benchs/` and use Google Benchmark. To run them:

```bash
cmake -S . -B build -DBUILD_BENCH=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --target benchs -j
# run the bench binary (binary name under build/bin or as configured)
build/bin/benchs --benchmark_min_time=0.1
```

## Common problems & troubleshooting

1. "LINK : fatal error LNK1104: cannot open file 'kernel32.lib'"
   - This means the Windows SDK paths are not configured. Launch a Visual Studio Developer Command Prompt (VsDevCmd) and re-run CMake.

2. Missing cmake package or library (find_package failure)
   - Ensure vcpkg manifest (`vcpkg.json`) lists the dependency and run `vcpkg install` or `vcpkg install --manifest` from the project root.

3. Build/timeouts or OOM when building large targets
   - Lower parallel job count (e.g. `cmake --build build -- -j4`) or increase machine resources.

## Developer tips

- Keep vcpkg updated and use manifest mode for reproducible dependencies.
- Run unit tests regularly and add tests for new features.
- For CI, use the same vcpkg baseline and toolchain file to keep builds reproducible.
