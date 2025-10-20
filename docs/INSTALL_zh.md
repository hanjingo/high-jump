# 安装与构建指南（Linux / macOS / Windows）

本文档涵盖在 Linux、macOS、Windows 平台为 `high-jump` 项目搭建开发环境的步骤，推荐使用 CMake + vcpkg（可移植且容易在 CI 中复现）。

## 前提

- CMake 3.19 以上（建议 3.21+）
- 支持 C++17 的编译器：
  - Linux：GCC 9+/11+ 或 Clang 9+/12+
  - macOS：Xcode 命令行工具（Clang）
  - Windows：Visual Studio 2019
- vcpkg（推荐）用于依赖管理
- Ninja（可选）用于加速构建

## 准备 vcpkg

1. 如果尚未安装 vcpkg，请克隆并引导安装：

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
```

Windows（PowerShell）：

```powershell
git clone https://github.com/microsoft/vcpkg.git $env:USERPROFILE\\vcpkg
cd $env:USERPROFILE\\vcpkg
.\bootstrap-vcpkg.bat
```

2. 在 CMake 中使用 vcpkg toolchain：

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

## Linux / macOS: 构建与测试（推荐流程）

```bash
# 在项目根目录
export VCPKG_ROOT=~/vcpkg
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build -- -j$(nproc)
ctest --test-dir build --output-on-failure
```

提示：
- 若遇到缺少包，请运行 `vcpkg install`（参考 `vcpkg.json`）或使用 `vcpkg install --manifest`。
- 需要 AddressSanitizer 时，可传入 `-DASAN=ON`（仅限支持的编译器）。

## Windows（Visual Studio 2019）

打开 "x64 本机工具命令提示符 (x64 Native Tools Command Prompt)"，或运行 `VsDevCmd.bat`，然后：

```powershell
# 在项目根目录
$env:VCPKG_ROOT = 'C:\path\to\vcpkg'
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\\scripts\\buildsystems\\vcpkg.cmake" -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug
```

Windows 重要提示：
- 请在 Visual Studio 开发者命令提示符中运行 CMake，这样 Windows SDK 与 MSVC 的环境变量（LIB / INCLUDE）才会被正确设置；否则可能遇到诸如 `kernel32.lib` 无法找到的链接错误。
- 使用 Visual Studio 生成器（`-G "Visual Studio 16 2019"`）将生成 solution 文件，可直接在 IDE 打开并编译。

## 运行基准测试

基准位于 `benchs/`，使用 Google Benchmark：

```bash
cmake -S . -B build -DBUILD_BENCH=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --target benchs -j
# 运行基准二进制（路径视构建配置而定）
build/bin/benchs --benchmark_min_time=0.1
```

## 常见问题与排查

1. "LINK : fatal error LNK1104: cannot open file 'kernel32.lib'"
   - 表示 Windows SDK 路径未配置。请使用 Visual Studio Developer Command Prompt（VsDevCmd）重新运行 CMake。

2. find_package / find_library 失败
   - 确认 `vcpkg.json` 中声明了依赖，并从项目根运行 `vcpkg install --manifest` 或手动 `vcpkg install`。

3. 构建内存或超时问题
   - 降低并行度例如 `-j4`，或在更大内存的机器上构建。

## 开发建议

- 在 CI 中使用相同的 vcpkg 基线（baseline）与 toolchain 文件以保证可重复构建。
- 为新功能撰写单元测试，并定期运行基准测试以捕获性能回归。
