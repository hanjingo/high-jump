# C++ 开发环境配置脚本

适用于 Windows、Linux 和 macOS 的跨平台 C++ 开发环境自动配置脚本。

## 概述

C++ 环境配置脚本（`cpp.sh` 和 `cpp.bat`）提供了跨不同操作系统的 C++ 开发工具的自动安装和配置。它们能够检测平台特定需求并安装适当的编译器、调试器和构建工具。

## 功能特性

- **跨平台支持**：Windows（cpp.bat）、Linux 和 macOS（cpp.sh）
- **多编译器支持**：GCC、Clang、MSVC、MinGW
- **灵活的工具选择**：可配置的调试器和构建系统
- **自动检测**：平台和现有工具检测
- **试运行模式**：预览安装步骤而不执行更改
- **全面验证**：安装后验证

## 脚本概述

### cpp.sh（Linux/macOS）
支持多种包管理器和 Homebrew 的 Unix 系统通用脚本。

### cpp.bat（Windows）
支持 Visual Studio、LLVM Clang 和 MinGW 安装的 Windows 批处理脚本。

## 使用方法

### Linux/macOS（cpp.sh）

#### 基本语法
```bash
./scripts/cpp.sh [选项]
```

#### 快速开始
```bash
# 安装 GCC 和 Clang 以及默认工具
./scripts/cpp.sh

# 安装特定编译器版本
./scripts/cpp.sh -c gcc -v 11

# 预览安装
./scripts/cpp.sh --dry-run
```

#### 选项

| 选项 | 简写 | 描述 | 默认值 |
|------|------|------|--------|
| `--compilers COMPILERS` | `-c` | 要安装的编译器（gcc,clang） | `gcc,clang` |
| `--version VERSION` | `-v` | 要安装的特定版本 | `latest` |
| `--tools TOOLS` | | 附加工具（gdb,lldb,ninja,make） | `gdb,ninja,make` |
| `--no-cmake` | | 跳过 CMake 安装 | |
| `--no-pkg-config` | | 跳过 pkg-config 安装 | |
| `--help` | `-h` | 显示帮助信息 | |
| `--dry-run` | | 显示将要执行的操作而不实际执行 | |
| `--verbose` | | 详细输出 | |

### Windows（cpp.bat）

#### 基本语法
```cmd
scripts\cpp.bat [选项]
```

#### 快速开始
```cmd
rem 安装 MSVC 和 Clang 以及默认工具
scripts\cpp.bat

rem 安装特定编译器
scripts\cpp.bat /compilers:msvc /version:2022

rem 预览安装
scripts\cpp.bat /dry-run
```

#### 选项

| 选项 | 描述 | 默认值 |
|------|------|--------|
| `/compilers:COMPILERS` | 要安装的编译器（msvc,clang,mingw） | `msvc,clang` |
| `/version:VERSION` | 要安装的特定版本 | `latest` |
| `/tools:TOOLS` | 附加工具（ninja,make,gdb,lldb） | `ninja,gdb` |
| `/no-cmake` | 跳过 CMake 安装 | |
| `/no-vcpkg` | 跳过 vcpkg 安装 | |
| `/h` 或 `/help` | 显示帮助信息 | |
| `/dry-run` | 显示将要执行的操作而不实际执行 | |
| `/verbose` | 详细输出 | |

## 支持的编译器

### GCC（GNU 编译器集合）
- **Linux**：通过包管理器（apt、yum、dnf、pacman）
- **macOS**：通过 Homebrew
- **Windows**：通过 MinGW 或 MSYS2

**特性**：
- 完整的 C++17/20 支持
- 广泛的优化选项
- 跨平台兼容性
- 庞大的生态系统支持

### Clang（LLVM）
- **Linux**：通过包管理器或 LLVM 发布版
- **macOS**：包含在 Xcode 命令行工具中
- **Windows**：通过 LLVM 发布版或 Visual Studio

**特性**：
- 现代 C++ 标准支持
- 更好的错误信息
- 静态分析工具
- 交叉编译支持

### MSVC（Microsoft Visual C++）
- **Windows**：通过 Visual Studio 2019/2022

**特性**：
- 原生 Windows 开发
- 出色的 Windows SDK 集成
- Visual Studio IDE 集成
- IntelliSense 支持

### MinGW（Windows 最小 GNU）
- **Windows**：通过 MSYS2 或独立安装程序

**特性**：
- Windows 上的 GCC 工具链
- POSIX 兼容层
- MSVC 的开源替代方案

## 支持的构建工具

### CMake
跨平台构建系统生成器
- **安装**：在所有平台上可用
- **特性**：现代 C++ 项目配置、依赖管理
- **集成**：与所有支持的编译器兼容

### Ninja
快速、小型的构建系统
- **安装**：通过包管理器可用
- **特性**：并行构建、增量编译
- **用途**：CMake 和其他构建生成器的后端

### Make
传统构建系统
- **安装**：Unix 系统标准配置，Windows 通过 MinGW 可用
- **特性**：基于 Makefile 的构建、广泛支持
- **兼容性**：与所有编译器兼容

## 支持的调试器

### GDB（GNU 调试器）
- **Linux**：通过包管理器标准安装
- **macOS**：通过 Homebrew 可用（需要代码签名）
- **Windows**：通过 MinGW/MSYS2

**特性**：
- 命令行和图形界面
- 远程调试支持
- 广泛的脚本功能

### LLDB（LLVM 调试器）
- **Linux**：通过包管理器
- **macOS**：包含在 Xcode 命令行工具中
- **Windows**：通过 LLVM 安装

**特性**：
- 现代调试界面
- Python 脚本支持
- 比 GDB 更好的 C++ 支持

## 平台特定详情

### Linux

#### Ubuntu/Debian
```bash
# 自动更新包列表
sudo apt-get update

# 支持的包
sudo apt-get install build-essential gcc g++ clang libc++-dev cmake ninja-build gdb
```

#### CentOS/RHEL/Fedora
```bash
# 使用 yum 或 dnf
sudo dnf install gcc gcc-c++ clang cmake ninja-build gdb
```

#### Arch Linux
```bash
# 使用 pacman
sudo pacman -S gcc clang cmake ninja gdb
```

### macOS

#### 要求
- Homebrew 包管理器
- Xcode 命令行工具

#### 安装
```bash
# 如果没有 Homebrew，先安装
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装开发工具
brew install gcc clang cmake ninja gdb
```

### Windows

#### Visual Studio 安装
- Visual Studio 2019/2022（社区版、专业版或企业版）
- 所需工作负载：
  - 使用 C++ 的桌面开发
  - MSVC v143 构建工具
  - Windows 10/11 SDK

#### 替代安装
- **Clang**：LLVM 发布版或通过 Visual Studio
- **MinGW**：MSYS2 或独立安装程序
- **CMake**：官方安装程序或包管理器
- **Ninja**：官方发布版或包管理器

## 示例

### 基本安装

#### Linux/macOS
```bash
# 默认安装
./scripts/cpp.sh

# 仅 GCC 特定版本
./scripts/cpp.sh -c gcc -v 11

# Clang 与 LLDB 调试器
./scripts/cpp.sh -c clang --tools lldb,ninja,make
```

#### Windows
```cmd
rem 默认安装
scripts\cpp.bat

rem 仅 MSVC 2022
scripts\cpp.bat /compilers:msvc /version:2022

rem 完整开发环境
scripts\cpp.bat /compilers:msvc,clang /tools:ninja,gdb,lldb
```

### 开发场景

#### 跨平台项目
```bash
# Linux/macOS
./scripts/cpp.sh -c gcc,clang --tools gdb,lldb,ninja

# Windows
scripts\cpp.bat /compilers:msvc,clang /tools:ninja,gdb
```

#### 性能优化
```bash
# 安装所有编译器进行比较
./scripts/cpp.sh -c gcc,clang --tools ninja

# Windows 等效
scripts\cpp.bat /compilers:msvc,clang,mingw /tools:ninja
```

#### 遗留支持
```bash
# 兼容性的特定 GCC 版本
./scripts/cpp.sh -c gcc -v 9 --tools make,gdb

# Windows 的 Visual Studio 2019
scripts\cpp.bat /compilers:msvc /version:2019
```

## 验证

### 安装后检查

#### 编译器验证
```bash
# GCC
gcc --version
g++ --version

# Clang
clang --version
clang++ --version

# Windows MSVC（在开发者命令提示符中）
cl.exe 2>&1 | findstr /c:"Microsoft"
```

#### 构建工具验证
```bash
# CMake
cmake --version

# Ninja
ninja --version

# Make
make --version
```

#### 调试器验证
```bash
# GDB
gdb --version

# LLDB
lldb --version
```

### 简单测试程序
```cpp
// test.cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

#### 编译和运行测试
```bash
# GCC
g++ -std=c++17 -o test_gcc test.cpp
./test_gcc

# Clang
clang++ -std=c++17 -o test_clang test.cpp
./test_clang

# Windows MSVC
cl /EHsc /std:c++17 test.cpp
test.exe
```

## 故障排除

### 常见问题

#### 找不到包管理器（Linux）
```bash
# 检查可用的包管理器
which apt-get yum dnf pacman

# 可能需要手动安装
```

#### 找不到 Homebrew（macOS）
```bash
# 先安装 Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### 找不到 Visual Studio（Windows）
- 从 https://visualstudio.microsoft.com/ 下载
- 安装"使用 C++ 的桌面开发"工作负载
- 确保选择了 MSVC 构建工具

#### 权限问题（Linux/macOS）
```bash
# 确保脚本可执行
chmod +x scripts/cpp.sh

# 使用适当的权限运行
sudo ./scripts/cpp.sh  # 如果需要
```

#### 路径问题（Windows）
```cmd
rem 刷新环境变量
refreshenv

rem 或重启命令提示符/PowerShell
```

### 调试模式
```bash
# 调试的详细输出
./scripts/cpp.sh --verbose

# 试运行查看计划的操作
./scripts/cpp.sh --dry-run
```

## 高级配置

### 环境变量

#### Linux/macOS
```bash
# 编译器首选项
export CC=gcc
export CXX=g++

# 或使用 Clang
export CC=clang
export CXX=clang++

# CMake 生成器
export CMAKE_GENERATOR=Ninja
```

#### Windows
```cmd
rem 设置编译器（在开发者命令提示符中）
set CC=cl
set CXX=cl

rem CMake 生成器
set CMAKE_GENERATOR=Ninja
```

### CMake 工具链文件
```cmake
# Linux/macOS 工具链示例
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)

# Windows 工具链示例
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_CXX_STANDARD 17)
```

## IDE 集成

### Visual Studio Code
- 安装 C/C++ 扩展
- 在设置中配置编译器路径
- 使用 CMake Tools 扩展

### CLion
- 自动编译器检测
- 集成 CMake 支持
- 内置调试器集成

### Visual Studio（Windows）
- 原生 MSVC 集成
- CMake 项目支持
- 集成调试和性能分析

## 性能考虑

- **并行编译**：使用 `-j` 标志与 make 或 ninja
- **优化标志**：发布构建使用 `-O2` 或 `-O3`
- **调试符号**：调试构建使用 `-g` 标志
- **静态分析**：启用编译器警告和静态分析工具

## 安全注意事项

- 脚本可能需要管理员/root 权限
- 执行前始终验证脚本来源
- 尽可能使用官方包仓库
- 保持编译器和工具更新以获取安全补丁

## 相关文档

- [GCC 文档](https://gcc.gnu.org/releases.html)
- [Clang 文档](https://clang.llvm.org/docs/)
- [CMake 文档](https://cmake.org/documentation/)
- [Visual Studio 文档](https://docs.microsoft.com/en-us/cpp/)