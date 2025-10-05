# Windows PowerShell script for C++ Development Environment Setup
# Usage: .\cpp.ps1 [OPTIONS]
# Options:
#   -c, --compilers COMPILERS     Compilers to install (msvc,mingw,clang) (default: msvc,mingw)
#   -v, --version VERSION         Specific version to install (default: latest)
#   --tools TOOLS                 Additional tools (gdb,lldb,ninja,make) (default: gdb,ninja,make)
#   --no-cmake                    Skip CMake installation
#   --no-pkg-config               Skip pkg-config installation
#   -h, --help                    Show this help message
#   --dry-run                     Show what would be done without executing
#   --verbose                     Verbose output

param(
    [string]$compilers = "msvc,mingw",
    [string]$version = "latest",
    [string]$tools = "gdb,ninja,make",
    [switch]$no_cmake,
    [switch]$no_pkg_config,
    [switch]$dry_run,
    [switch]$verbose,
    [switch]$help
)

function Show-Help {
    Write-Host @"
C++ Development Environment Setup Script (Windows)

USAGE:
    .\cpp.ps1 [OPTIONS]

OPTIONS:
    -c, --compilers COMPILERS     Compilers to install (msvc,mingw,clang) (default: msvc,mingw)
    -v, --version VERSION         Specific version to install (default: latest)
    --tools TOOLS                 Additional tools (gdb,lldb,ninja,make) (default: gdb,ninja,make)
    --no-cmake                    Skip CMake installation
    --no-pkg-config               Skip pkg-config installation
    -h, --help                    Show this help message
    --dry-run                     Show what would be done without executing
    --verbose                     Verbose output

EXAMPLES:
    .\cpp.ps1                                  # Install MSVC and MinGW with default tools
    .\cpp.ps1 -c msvc -v 2022                  # Install MSVC 2022 only
    .\cpp.ps1 -c msvc,mingw --tools gdb,ninja  # Install both compilers with specific tools
    .\cpp.ps1 --dry-run                        # Preview installation without executing

SUPPORTED COMPILERS:
    - MSVC (Visual Studio Build Tools)
    - MinGW (GCC for Windows)
    - Clang (LLVM for Windows)

SUPPORTED TOOLS:
    - GDB (GNU Debugger)
    - LLDB (LLVM Debugger)
    - Ninja (Build system)
    - Make (Build system)
    - CMake (Build system generator)
    - pkg-config (Package configuration tool)
"@
}

if ($help) {
    Show-Help
    exit 0
}

function Log-Info($msg) { Write-Host "[INFO] $msg" }
function Log-Verbose($msg) { if ($verbose) { Write-Host "[VERBOSE] $msg" } }
function Log-Error($msg) { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Execute-Cmd($cmd) {
    if ($dry_run) {
        Write-Host "[DRY-RUN] Would execute: $cmd"
    } else {
        Log-Verbose "Executing: $cmd"
        Invoke-Expression $cmd
    }
}

# Convert comma-separated lists to arrays
$COMPILER_ARRAY = $compilers.Split(",")
$TOOLS_ARRAY = $tools.Split(",")

Log-Info "==== C++ Development Environment Setup Script ===="
Log-Info "Compilers: $compilers"
Log-Info "Version: $version"
Log-Info "Tools: $tools"
Log-Info "CMake: $(if ($no_cmake) { 'Skip' } else { 'Yes' })"
Log-Info "pkg-config: $(if ($no_pkg_config) { 'Skip' } else { 'Yes' })"
if ($dry_run) { Log-Info "Mode: DRY RUN (simulation only)" }
Log-Info "================================================="

# Function to install MSVC (Visual Studio Build Tools)
function Install-MSVC {
    Log-Info "Installing MSVC (Visual Studio Build Tools)..."
    $vsInstaller = "$env:ProgramFiles(x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
    if (-not (Test-Path $vsInstaller)) {
        Log-Info "Downloading Visual Studio Build Tools installer..."
        $url = "https://aka.ms/vs/17/release/vs_BuildTools.exe"
        $installer = "$env:TEMP\vs_BuildTools.exe"
        Execute-Cmd "Invoke-WebRequest -Uri '$url' -OutFile '$installer'"
        $vsInstaller = $installer
    }
    $vsCmd = "$vsInstaller --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --passive --norestart"
    Execute-Cmd $vsCmd
}

# Function to install MinGW
function Install-MinGW {
    Log-Info "Installing MinGW-w64..."
    $mingwUrl = "https://sourceforge.net/projects/mingw-w64/files/latest/download"
    $mingwInstaller = "$env:TEMP\mingw-w64.exe"
    Execute-Cmd "Invoke-WebRequest -Uri '$mingwUrl' -OutFile '$mingwInstaller'"
    Execute-Cmd "$mingwInstaller"
}

# Function to install LLVM/Clang
function Install-Clang {
    Log-Info "Installing LLVM/Clang..."
    $clangUrl = "https://github.com/llvm/llvm-project/releases/latest/download/LLVM-win64.exe"
    $clangInstaller = "$env:TEMP\LLVM-win64.exe"
    Execute-Cmd "Invoke-WebRequest -Uri '$clangUrl' -OutFile '$clangInstaller'"
    Execute-Cmd "$clangInstaller"
}

# Function to install tools via Chocolatey
function Install-ChocoTool($tool) {
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Log-Info "Installing Chocolatey package manager..."
        Execute-Cmd "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
    }
    Log-Info "Installing $tool via Chocolatey..."
    Execute-Cmd "choco install $tool -y"
}

# Function to install CMake
function Install-CMake {
    Log-Info "Installing CMake..."
    Install-ChocoTool "cmake"
}

# Function to install pkg-config
function Install-PkgConfig {
    Log-Info "Installing pkg-config..."
    Install-ChocoTool "pkgconfiglite"
}

# Function to install build/debug tools
function Install-Tools {
    foreach ($tool in $TOOLS_ARRAY) {
        switch ($tool) {
            "gdb" { Install-ChocoTool "gdb" }
            "lldb" { Install-ChocoTool "lldb" }
            "ninja" { Install-ChocoTool "ninja" }
            "make" { Install-ChocoTool "make" }
        }
    }
    if (-not $no_cmake) { Install-CMake }
    if (-not $no_pkg_config) { Install-PkgConfig }
}

# Function to verify installation
function Verify-Installation {
    Log-Info "Verifying installation..."
    foreach ($compiler in $COMPILER_ARRAY) {
        switch ($compiler) {
            "msvc" {
                if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
                    Log-Info "✓ MSVC installed: $(cl.exe 2>&1 | Select-String 'Version')"
                } else {
                    Log-Error "✗ MSVC not found"
                }
            }
            "mingw" {
                if (Get-Command gcc -ErrorAction SilentlyContinue) {
                    Log-Info "✓ MinGW GCC installed: $(gcc --version | Select-Object -First 1)"
                } else {
                    Log-Error "✗ MinGW GCC not found"
                }
            }
            "clang" {
                if (Get-Command clang -ErrorAction SilentlyContinue) {
                    Log-Info "✓ Clang installed: $(clang --version | Select-Object -First 1)"
                } else {
                    Log-Error "✗ Clang not found"
                }
            }
        }
    }
    foreach ($tool in $TOOLS_ARRAY) {
        if (Get-Command $tool -ErrorAction SilentlyContinue) {
            Log-Info "✓ $tool installed: $($(& $tool --version | Select-Object -First 1))"
        } else {
            Log-Error "✗ $tool not found"
        }
    }
    if (-not $no_cmake) {
        if (Get-Command cmake -ErrorAction SilentlyContinue) {
            Log-Info "✓ CMake installed: $(cmake --version | Select-Object -First 1)"
        } else {
            Log-Error "✗ CMake not found"
        }
    }
    if (-not $no_pkg_config) {
        if (Get-Command pkg-config -ErrorAction SilentlyContinue) {
            Log-Info "✓ pkg-config installed: $(pkg-config --version)"
        } else {
            Log-Error "✗ pkg-config not found"
        }
    }
}

# Main installation logic
function Main {
    Log-Info "Starting C++ development environment setup..."
    Log-Info "Target OS: Windows ($env:PROCESSOR_ARCHITECTURE)"
    foreach ($compiler in $COMPILER_ARRAY) {
        switch ($compiler) {
            "msvc" { Install-MSVC }
            "mingw" { Install-MinGW }
            "clang" { Install-Clang }
            default { Log-Error "Unknown compiler: $compiler" }
        }
    }
    Install-Tools
    if (-not $dry_run) {
        Log-Info "Installation completed!"
        Verify-Installation
        Log-Info "Quick test commands:"
        foreach ($compiler in $COMPILER_ARRAY) {
            switch ($compiler) {
                "msvc" { Write-Host "  cl.exe /?" }
                "mingw" { Write-Host "  gcc --version && g++ --version" }
                "clang" { Write-Host "  clang --version && clang++ --version" }
            }
        }
        Write-Host "  cmake --version"
    } else {
        Log-Info "Dry run completed - no changes were made"
    }
}

Main

Log-Info "==== C++ development environment setup finished ====="
