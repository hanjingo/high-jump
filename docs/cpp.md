# C++ Development Environment Setup Scripts

Cross-platform C++ development environment automatic configuration scripts for Windows, Linux, and macOS.

## Overview

The C++ environment setup scripts (`cpp.sh` and `cpp.bat`) provide automated installation and configuration of essential C++ development tools across different operating systems. They detect platform-specific requirements and install appropriate compilers, debuggers, and build tools.

## Features

- **Cross-platform support**: Windows (cpp.bat), Linux and macOS (cpp.sh)
- **Multiple compiler support**: GCC, Clang, MSVC, MinGW
- **Flexible tool selection**: Configurable debuggers and build systems
- **Automatic detection**: Platform and existing tool detection
- **Dry-run mode**: Preview installation steps without making changes
- **Comprehensive verification**: Post-installation validation

## Scripts Overview

### cpp.sh (Linux/macOS)
Universal script for Unix-like systems supporting multiple package managers and Homebrew.

### cpp.bat (Windows) 
Batch script for Windows supporting Visual Studio, LLVM Clang, and MinGW installations.

## Usage

### Linux/macOS (cpp.sh)

#### Basic Syntax
```bash
./scripts/cpp.sh [OPTIONS]
```

#### Quick Start
```bash
# Install GCC and Clang with default tools
./scripts/cpp.sh

# Install specific compiler version
./scripts/cpp.sh -c gcc -v 11

# Preview installation
./scripts/cpp.sh --dry-run
```

#### Options

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--compilers COMPILERS` | `-c` | Compilers to install (gcc,clang) | `gcc,clang` |
| `--version VERSION` | `-v` | Specific version to install | `latest` |
| `--tools TOOLS` | | Additional tools (gdb,lldb,ninja,make) | `gdb,ninja,make` |
| `--no-cmake` | | Skip CMake installation | |
| `--no-pkg-config` | | Skip pkg-config installation | |
| `--help` | `-h` | Show help message | |
| `--dry-run` | | Show what would be done without executing | |
| `--verbose` | | Verbose output | |

### Windows (cpp.bat)

#### Basic Syntax
```cmd
scripts\cpp.bat [OPTIONS]
```

#### Quick Start
```cmd
rem Install MSVC and Clang with default tools
scripts\cpp.bat

rem Install specific compiler
scripts\cpp.bat /compilers:msvc /version:2022

rem Preview installation
scripts\cpp.bat /dry-run
```

#### Options

| Option | Description | Default |
|--------|-------------|---------|
| `/compilers:COMPILERS` | Compilers to install (msvc,clang,mingw) | `msvc,clang` |
| `/version:VERSION` | Specific version to install | `latest` |
| `/tools:TOOLS` | Additional tools (ninja,make,gdb,lldb) | `ninja,gdb` |
| `/no-cmake` | Skip CMake installation | |
| `/no-vcpkg` | Skip vcpkg installation | |
| `/h` or `/help` | Show help message | |
| `/dry-run` | Show what would be done without executing | |
| `/verbose` | Verbose output | |

## Supported Compilers

### GCC (GNU Compiler Collection)
- **Linux**: Via package managers (apt, yum, dnf, pacman)
- **macOS**: Via Homebrew
- **Windows**: Via MinGW or MSYS2

**Features**:
- Full C++17/20 support
- Extensive optimization options
- Cross-platform compatibility
- Large ecosystem support

### Clang (LLVM)
- **Linux**: Via package managers or LLVM releases
- **macOS**: Included with Xcode Command Line Tools
- **Windows**: Via LLVM releases or Visual Studio

**Features**:
- Modern C++ standards support
- Better error messages
- Static analysis tools
- Cross-compilation support

### MSVC (Microsoft Visual C++)
- **Windows**: Via Visual Studio 2019/2022

**Features**:
- Native Windows development
- Excellent Windows SDK integration
- Visual Studio IDE integration
- IntelliSense support

### MinGW (Minimalist GNU for Windows)
- **Windows**: Via MSYS2 or standalone installers

**Features**:
- GCC toolchain on Windows
- POSIX compatibility layer
- Open source alternative to MSVC

## Supported Build Tools

### CMake
Cross-platform build system generator
- **Installation**: Available on all platforms
- **Features**: Modern C++ project configuration, dependency management
- **Integration**: Works with all supported compilers

### Ninja
Fast, small build system
- **Installation**: Available via package managers
- **Features**: Parallel builds, incremental compilation
- **Use case**: Backend for CMake and other build generators

### Make
Traditional build system
- **Installation**: Standard on Unix systems, available via MinGW on Windows
- **Features**: Makefile-based builds, widely supported
- **Compatibility**: Works with all compilers

## Supported Debuggers

### GDB (GNU Debugger)
- **Linux**: Standard installation via package managers
- **macOS**: Available via Homebrew (requires code signing)
- **Windows**: Via MinGW/MSYS2

**Features**:
- Command-line and GUI interfaces
- Remote debugging support
- Extensive scripting capabilities

### LLDB (LLVM Debugger)
- **Linux**: Via package managers
- **macOS**: Included with Xcode Command Line Tools
- **Windows**: Via LLVM installation

**Features**:
- Modern debugging interface
- Python scripting support
- Better C++ support than GDB

## Platform-Specific Details

### Linux

#### Ubuntu/Debian
```bash
# Update package lists automatically
sudo apt-get update

# Supported packages
sudo apt-get install build-essential gcc g++ clang libc++-dev cmake ninja-build gdb
```

#### CentOS/RHEL/Fedora
```bash
# Uses yum or dnf
sudo dnf install gcc gcc-c++ clang cmake ninja-build gdb
```

#### Arch Linux
```bash
# Uses pacman
sudo pacman -S gcc clang cmake ninja gdb
```

### macOS

#### Requirements
- Homebrew package manager
- Xcode Command Line Tools

#### Installation
```bash
# Install Homebrew if not present
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install development tools
brew install gcc clang cmake ninja gdb
```

### Windows

#### Visual Studio Installation
- Visual Studio 2019/2022 (Community, Professional, or Enterprise)
- Required workloads:
  - Desktop development with C++
  - MSVC v143 build tools
  - Windows 10/11 SDK

#### Alternative Installations
- **Clang**: LLVM releases or via Visual Studio
- **MinGW**: MSYS2 or standalone installers
- **CMake**: Official installer or package managers
- **Ninja**: Official releases or package managers

## Examples

### Basic Installation

#### Linux/macOS
```bash
# Default installation
./scripts/cpp.sh

# GCC only with specific version
./scripts/cpp.sh -c gcc -v 11

# Clang with LLDB debugger
./scripts/cpp.sh -c clang --tools lldb,ninja,make
```

#### Windows
```cmd
rem Default installation
scripts\cpp.bat

rem MSVC 2022 only
scripts\cpp.bat /compilers:msvc /version:2022

rem Full development environment
scripts\cpp.bat /compilers:msvc,clang /tools:ninja,gdb,lldb
```

### Development Scenarios

#### Cross-Platform Project
```bash
# Linux/macOS
./scripts/cpp.sh -c gcc,clang --tools gdb,lldb,ninja

# Windows
scripts\cpp.bat /compilers:msvc,clang /tools:ninja,gdb
```

#### Performance Optimization
```bash
# Install all compilers for comparison
./scripts/cpp.sh -c gcc,clang --tools ninja

# Windows equivalent
scripts\cpp.bat /compilers:msvc,clang,mingw /tools:ninja
```

#### Legacy Support
```bash
# Specific GCC version for compatibility
./scripts/cpp.sh -c gcc -v 9 --tools make,gdb

# Visual Studio 2019 for Windows
scripts\cpp.bat /compilers:msvc /version:2019
```

## Verification

### Post-Installation Checks

#### Compiler Verification
```bash
# GCC
gcc --version
g++ --version

# Clang
clang --version
clang++ --version

# Windows MSVC (in Developer Command Prompt)
cl.exe 2>&1 | findstr /c:"Microsoft"
```

#### Build Tools Verification
```bash
# CMake
cmake --version

# Ninja
ninja --version

# Make
make --version
```

#### Debugger Verification
```bash
# GDB
gdb --version

# LLDB
lldb --version
```

### Simple Test Program
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

#### Compile and Run Tests
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

## Troubleshooting

### Common Issues

#### Package Manager Not Found (Linux)
```bash
# Check available package managers
which apt-get yum dnf pacman

# Manual installation may be required
```

#### Homebrew Not Found (macOS)
```bash
# Install Homebrew first
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### Visual Studio Not Found (Windows)
- Download from https://visualstudio.microsoft.com/
- Install "Desktop development with C++" workload
- Ensure MSVC build tools are selected

#### Permission Issues (Linux/macOS)
```bash
# Ensure script is executable
chmod +x scripts/cpp.sh

# Run with appropriate privileges
sudo ./scripts/cpp.sh  # if needed
```

#### Path Issues (Windows)
```cmd
rem Refresh environment variables
refreshenv

rem Or restart Command Prompt/PowerShell
```

### Debug Mode
```bash
# Verbose output for debugging
./scripts/cpp.sh --verbose

# Dry run to see planned actions
./scripts/cpp.sh --dry-run
```

## Advanced Configuration

### Environment Variables

#### Linux/macOS
```bash
# Compiler preferences
export CC=gcc
export CXX=g++

# or for Clang
export CC=clang
export CXX=clang++

# CMake generator
export CMAKE_GENERATOR=Ninja
```

#### Windows
```cmd
rem Set compiler (in Developer Command Prompt)
set CC=cl
set CXX=cl

rem CMake generator
set CMAKE_GENERATOR=Ninja
```

### CMake Toolchain Files
```cmake
# Linux/macOS toolchain example
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)

# Windows toolchain example
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_CXX_STANDARD 17)
```

## Integration with IDEs

### Visual Studio Code
- Install C/C++ extension
- Configure compiler paths in settings
- Use CMake Tools extension

### CLion
- Automatic compiler detection
- Integrated CMake support
- Built-in debugger integration

### Visual Studio (Windows)
- Native MSVC integration
- CMake project support
- Integrated debugging and profiling

## Performance Considerations

- **Parallel compilation**: Use `-j` flag with make or ninja
- **Optimization flags**: `-O2` or `-O3` for release builds
- **Debug symbols**: `-g` flag for debugging builds
- **Static analysis**: Enable compiler warnings and static analysis tools

## Security Notes

- Scripts may require administrator/root privileges
- Always verify script sources before execution
- Use official package repositories when possible
- Keep compilers and tools updated for security patches

## Related Documentation

- [GCC Documentation](https://gcc.gnu.org/releases.html)
- [Clang Documentation](https://clang.llvm.org/docs/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Visual Studio Documentation](https://docs.microsoft.com/en-us/cpp/)