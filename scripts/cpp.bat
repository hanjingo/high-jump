@echo off
setlocal enabledelayedexpansion

rem Cross-platform C++ Development Environment Setup Script for Windows
rem Usage: cpp.bat [OPTIONS]
rem Options:
rem   /compilers:COMPILERS     Compilers to install (msvc,clang,mingw) (default: msvc,clang)
rem   /version:VERSION         Specific version to install (default: latest)
rem   /tools:TOOLS             Additional tools (ninja,make,gdb,lldb) (default: ninja,gdb)
rem   /no-cmake                Skip CMake installation
rem   /no-vcpkg                Skip vcpkg installation
rem   /h or /help              Show this help message
rem   /dry-run                 Show what would be done without executing
rem   /verbose                 Verbose output

rem Default values
set "DEFAULT_COMPILERS=msvc,clang"
set "DEFAULT_VERSION=latest"
set "DEFAULT_TOOLS=ninja,gdb"

rem Initialize variables
set "COMPILERS=%DEFAULT_COMPILERS%"
set "VERSION=%DEFAULT_VERSION%"
set "TOOLS=%DEFAULT_TOOLS%"
set "INSTALL_CMAKE=true"
set "INSTALL_VCPKG=true"
set "DRY_RUN=false"
set "VERBOSE=false"

rem Function to show help
:show_help
echo Cross-platform C++ Development Environment Setup Script for Windows
echo.
echo USAGE:
echo     %~nx0 [OPTIONS]
echo.
echo OPTIONS:
echo     /compilers:COMPILERS     Compilers to install (msvc,clang,mingw) (default: %DEFAULT_COMPILERS%)
echo     /version:VERSION         Specific version to install (default: %DEFAULT_VERSION%)
echo     /tools:TOOLS             Additional tools (ninja,make,gdb,lldb) (default: %DEFAULT_TOOLS%)
echo     /no-cmake                Skip CMake installation
echo     /no-vcpkg                Skip vcpkg installation
echo     /h or /help              Show this help message
echo     /dry-run                 Show what would be done without executing
echo     /verbose                 Verbose output
echo.
echo EXAMPLES:
echo     %~nx0                                          # Install MSVC and Clang with default tools
echo     %~nx0 /compilers:msvc /version:2022           # Install MSVC 2022 only
echo     %~nx0 /compilers:msvc,clang /tools:ninja,gdb  # Install both compilers with specific tools
echo     %~nx0 /dry-run                                # Preview installation without executing
echo.
echo SUPPORTED COMPILERS:
echo     - MSVC (Microsoft Visual C++)
echo     - Clang (LLVM Clang)
echo     - MinGW (Minimalist GNU for Windows)
echo.
echo SUPPORTED TOOLS:
echo     - Ninja (Build system)
echo     - Make (Build system)
echo     - GDB (GNU Debugger)
echo     - LLDB (LLVM Debugger)
echo     - CMake (Build system generator)
echo     - vcpkg (Package manager)
echo.
echo NOTE:
echo     This script requires administrator privileges for some installations.
goto :eof

rem Parse command line arguments
:parse_args
if "%~1"=="" goto :start_main
if /i "%~1"=="/compilers" (
    set "COMPILERS=%~2"
    shift & shift & goto :parse_args
)
if "%~1" NEQ "" (
    for /f "tokens=1,2 delims=:" %%a in ("%~1") do (
        if /i "%%a"=="/compilers" set "COMPILERS=%%b"
        if /i "%%a"=="/version" set "VERSION=%%b"
        if /i "%%a"=="/tools" set "TOOLS=%%b"
    )
)
if /i "%~1"=="/no-cmake" (
    set "INSTALL_CMAKE=false"
    shift & goto :parse_args
)
if /i "%~1"=="/no-vcpkg" (
    set "INSTALL_VCPKG=false"
    shift & goto :parse_args
)
if /i "%~1"=="/dry-run" (
    set "DRY_RUN=true"
    shift & goto :parse_args
)
if /i "%~1"=="/verbose" (
    set "VERBOSE=true"
    shift & goto :parse_args
)
if /i "%~1"=="/h" goto :show_help
if /i "%~1"=="/help" goto :show_help
echo [ERROR] Unknown option: %~1
goto :show_help

shift & goto :parse_args

:start_main
echo ==== C++ Development Environment Setup Script for Windows ====
echo Compilers: %COMPILERS%
echo Version: %VERSION%
echo Tools: %TOOLS%
if "%INSTALL_CMAKE%"=="true" (echo CMake: Yes) else (echo CMake: Skip)
if "%INSTALL_VCPKG%"=="true" (echo vcpkg: Yes) else (echo vcpkg: Skip)
if "%DRY_RUN%"=="true" echo Mode: DRY RUN (simulation only)
echo ================================================================

rem Function to log actions
goto :main

:log_action
echo [INFO] %~1
goto :eof

:log_verbose
if "%VERBOSE%"=="true" echo [VERBOSE] %~1
goto :eof

:log_error
echo [ERROR] %~1 >&2
goto :eof

rem Function to execute or simulate command
:execute_cmd
if "%DRY_RUN%"=="true" (
    echo [DRY-RUN] Would execute: %*
) else (
    call :log_verbose "Executing: %*"
    %*
)
goto :eof

rem Function to check if command exists
:check_command_exists
where "%~1" >nul 2>&1
goto :eof

rem Function to detect Visual Studio installations
:detect_visual_studio
set "VS_FOUND=false"
set "VS_PATH="
set "VCVARS_PATH="

rem Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2022 Enterprise"
    goto :eof
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2022 Professional"
    goto :eof
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2022 Community"
    goto :eof
)

rem Visual Studio 2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise"
    set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2019 Enterprise"
    goto :eof
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"
    set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2019 Professional"
    goto :eof
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
    set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_FOUND=true"
    set "VS_VERSION=2019 Community"
    goto :eof
)
goto :eof

rem Function to install MSVC
:install_msvc
call :log_action "Checking MSVC installation..."

call :detect_visual_studio
if "%VS_FOUND%"=="true" (
    call :log_action "✓ Visual Studio found: %VS_VERSION%"
    call :log_action "  Path: %VS_PATH%"
    goto :eof
) else (
    call :log_error "✗ Visual Studio 2019/2022 not found"
    call :log_action "Please install Visual Studio with C++ development tools:"
    call :log_action "  - Visual Studio 2022: https://visualstudio.microsoft.com/vs/"
    call :log_action "  - Visual Studio 2019: https://visualstudio.microsoft.com/vs/older-downloads/"
    call :log_action "Required workloads:"
    call :log_action "  - Desktop development with C++"
    call :log_action "  - MSVC v143 - VS 2022 C++ x64/x86 build tools"
    call :log_action "  - Windows 10/11 SDK"
)
goto :eof

rem Function to install Clang
:install_clang
call :log_action "Installing Clang..."

rem Check if already installed
call :check_command_exists clang
if errorlevel 1 (
    call :log_action "Clang not found, installing via LLVM..."
    if "%DRY_RUN%"=="true" (
        echo [DRY-RUN] Would download and install LLVM Clang
    ) else (
        call :log_action "Please download and install LLVM from:"
        call :log_action "  https://github.com/llvm/llvm-project/releases"
        call :log_action "Or use Chocolatey: choco install llvm"
        call :log_action "Or use winget: winget install LLVM.LLVM"
    )
) else (
    for /f "tokens=*" %%i in ('clang --version 2^>nul ^| findstr /c:"clang version"') do (
        call :log_action "✓ Clang already installed: %%i"
    )
)
goto :eof

rem Function to install MinGW
:install_mingw
call :log_action "Installing MinGW..."

call :check_command_exists gcc
if errorlevel 1 (
    call :log_action "MinGW not found, installing..."
    if "%DRY_RUN%"=="true" (
        echo [DRY-RUN] Would download and install MinGW-w64
    ) else (
        call :log_action "Please install MinGW-w64:"
        call :log_action "  - MSYS2: https://www.msys2.org/"
        call :log_action "  - Chocolatey: choco install mingw"
        call :log_action "  - winget: winget install msys2.msys2"
    )
) else (
    for /f "tokens=*" %%i in ('gcc --version 2^>nul ^| findstr /c:"gcc"') do (
        call :log_action "✓ MinGW already installed: %%i"
    )
)
goto :eof

rem Function to install build tools
:install_build_tools
call :log_action "Installing build tools..."

rem Split tools by comma and process each
for %%t in (%TOOLS%) do (
    if /i "%%t"=="ninja" (
        call :check_command_exists ninja
        if errorlevel 1 (
            if "%DRY_RUN%"=="true" (
                echo [DRY-RUN] Would install Ninja build system
            ) else (
                call :log_action "Installing Ninja build system..."
                call :log_action "Download from: https://github.com/ninja-build/ninja/releases"
                call :log_action "Or use: choco install ninja"
                call :log_action "Or use: winget install Ninja-build.Ninja"
            )
        ) else (
            call :log_action "✓ Ninja already installed"
        )
    )
    
    if /i "%%t"=="make" (
        call :check_command_exists make
        if errorlevel 1 (
            if "%DRY_RUN%"=="true" (
                echo [DRY-RUN] Would install Make
            ) else (
                call :log_action "Installing Make..."
                call :log_action "Available via MSYS2 or MinGW installation"
            )
        ) else (
            call :log_action "✓ Make already installed"
        )
    )
    
    if /i "%%t"=="gdb" (
        call :check_command_exists gdb
        if errorlevel 1 (
            if "%DRY_RUN%"=="true" (
                echo [DRY-RUN] Would install GDB
            ) else (
                call :log_action "Installing GDB..."
                call :log_action "Available via MSYS2 or MinGW installation"
            )
        ) else (
            call :log_action "✓ GDB already installed"
        )
    )
    
    if /i "%%t"=="lldb" (
        call :check_command_exists lldb
        if errorlevel 1 (
            if "%DRY_RUN%"=="true" (
                echo [DRY-RUN] Would install LLDB
            ) else (
                call :log_action "LLDB is included with LLVM installation"
            )
        ) else (
            call :log_action "✓ LLDB already installed"
        )
    )
)

rem Install CMake
if "%INSTALL_CMAKE%"=="true" (
    call :check_command_exists cmake
    if errorlevel 1 (
        if "%DRY_RUN%"=="true" (
            echo [DRY-RUN] Would install CMake
        ) else (
            call :log_action "Installing CMake..."
            call :log_action "Download from: https://cmake.org/download/"
            call :log_action "Or use: choco install cmake"
            call :log_action "Or use: winget install Kitware.CMake"
        )
    ) else (
        for /f "tokens=*" %%i in ('cmake --version 2^>nul ^| findstr /c:"cmake version"') do (
            call :log_action "✓ CMake already installed: %%i"
        )
    )
)

rem Install vcpkg
if "%INSTALL_VCPKG%"=="true" (
    if exist "tools\vcpkg\vcpkg.exe" (
        call :log_action "✓ vcpkg already available in tools/vcpkg/"
    ) else (
        if "%DRY_RUN%"=="true" (
            echo [DRY-RUN] Would clone and bootstrap vcpkg
        ) else (
            call :log_action "Installing vcpkg..."
            if not exist "tools" mkdir tools
            cd tools
            git clone https://github.com/Microsoft/vcpkg.git
            cd vcpkg
            call .\bootstrap-vcpkg.bat
            cd ..\..
            call :log_action "✓ vcpkg installed in tools/vcpkg/"
        )
    )
)
goto :eof

rem Function to verify installation
:verify_installation
call :log_action "Verifying installation..."

set "VERIFICATION_FAILED=false"

rem Split compilers by comma and check each
for %%c in (%COMPILERS%) do (
    if /i "%%c"=="msvc" (
        call :detect_visual_studio
        if "%VS_FOUND%"=="true" (
            call :log_action "✓ MSVC available: %VS_VERSION%"
        ) else (
            call :log_error "✗ MSVC not found"
            set "VERIFICATION_FAILED=true"
        )
    )
    
    if /i "%%c"=="clang" (
        call :check_command_exists clang
        if not errorlevel 1 (
            for /f "tokens=*" %%i in ('clang --version 2^>nul ^| findstr /c:"clang version"') do (
                call :log_action "✓ Clang installed: %%i"
            )
        ) else (
            call :log_error "✗ Clang not found"
            set "VERIFICATION_FAILED=true"
        )
    )
    
    if /i "%%c"=="mingw" (
        call :check_command_exists gcc
        if not errorlevel 1 (
            for /f "tokens=*" %%i in ('gcc --version 2^>nul ^| findstr /c:"gcc"') do (
                call :log_action "✓ MinGW installed: %%i"
            )
        ) else (
            call :log_error "✗ MinGW not found"
            set "VERIFICATION_FAILED=true"
        )
    )
)

rem Check tools
for %%t in (%TOOLS%) do (
    call :check_command_exists %%t
    if not errorlevel 1 (
        call :log_action "✓ %%t installed"
    ) else (
        call :log_error "✗ %%t not found"
        set "VERIFICATION_FAILED=true"
    )
)

rem Check CMake
if "%INSTALL_CMAKE%"=="true" (
    call :check_command_exists cmake
    if not errorlevel 1 (
        for /f "tokens=*" %%i in ('cmake --version 2^>nul ^| findstr /c:"cmake version"') do (
            call :log_action "✓ CMake installed: %%i"
        )
    ) else (
        call :log_error "✗ CMake not found"
        set "VERIFICATION_FAILED=true"
    )
)

rem Check vcpkg
if "%INSTALL_VCPKG%"=="true" (
    if exist "tools\vcpkg\vcpkg.exe" (
        call :log_action "✓ vcpkg available"
    ) else (
        call :log_error "✗ vcpkg not found"
        set "VERIFICATION_FAILED=true"
    )
)

if "%VERIFICATION_FAILED%"=="false" (
    call :log_action "✓ All tools verified successfully!"
) else (
    call :log_error "✗ Some tools failed verification"
    exit /b 1
)
goto :eof

rem Main function
:main
call %*
call :parse_args %*

call :log_action "Starting C++ development environment setup for Windows..."
call :log_action "Target OS: Windows"

rem Process compilers
for %%c in (%COMPILERS%) do (
    if /i "%%c"=="msvc" call :install_msvc
    if /i "%%c"=="clang" call :install_clang
    if /i "%%c"=="mingw" call :install_mingw
    if not "%%c"=="msvc" if not "%%c"=="clang" if not "%%c"=="mingw" (
        call :log_error "Unknown compiler: %%c"
    )
)

rem Install build tools
call :install_build_tools

if "%DRY_RUN%"=="false" (
    call :log_action "Installation completed!"
    call :verify_installation
    
    call :log_action "Quick test commands:"
    for %%c in (%COMPILERS%) do (
        if /i "%%c"=="msvc" (
            echo   cl.exe 2^>^&1 ^| findstr /c:"Microsoft"
        )
        if /i "%%c"=="clang" (
            echo   clang --version
        )
        if /i "%%c"=="mingw" (
            echo   gcc --version
        )
    )
    echo   cmake --version
) else (
    call :log_action "Dry run completed - no changes were made"
)

echo ==== C++ development environment setup finished =====
exit /b 0