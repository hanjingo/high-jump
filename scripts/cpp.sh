#!/usr/bin/env bash

# Cross-platform C++ Development Environment Setup Script
# Usage: ./cpp.sh [OPTIONS]
# Options:
#   -c, --compilers COMPILERS     Compilers to install (gcc,clang,msvc) (default: gcc,clang)
#   -v, --version VERSION         Specific version to install (default: latest)
#   --tools TOOLS                 Additional tools (gdb,lldb,ninja,make) (default: gdb,ninja,make)
#   --no-cmake                    Skip CMake installation
#   --no-pkg-config               Skip pkg-config installation
#   -h, --help                    Show this help message
#   --dry-run                     Show what would be done without executing
#   --verbose                     Verbose output

set -e

# Default values
DEFAULT_COMPILERS="gcc,clang"
DEFAULT_VERSION="latest"
DEFAULT_TOOLS="gdb,ninja,make"

# Initialize variables
COMPILERS="$DEFAULT_COMPILERS"
VERSION="$DEFAULT_VERSION"
TOOLS="$DEFAULT_TOOLS"
INSTALL_CMAKE=true
INSTALL_PKG_CONFIG=true
DRY_RUN=false
VERBOSE=false

# Function to show help
show_help() {
    cat << EOF
Cross-platform C++ Development Environment Setup Script

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -c, --compilers COMPILERS     Compilers to install (gcc,clang,msvc) (default: $DEFAULT_COMPILERS)
    -v, --version VERSION         Specific version to install (default: $DEFAULT_VERSION)
    --tools TOOLS                 Additional tools (gdb,lldb,ninja,make) (default: $DEFAULT_TOOLS)
    --no-cmake                    Skip CMake installation
    --no-pkg-config               Skip pkg-config installation
    -h, --help                    Show this help message
    --dry-run                     Show what would be done without executing
    --verbose                     Verbose output

EXAMPLES:
    $0                                          # Install GCC and Clang with default tools
    $0 -c gcc -v 11                            # Install GCC 11 only
    $0 -c gcc,clang --tools gdb,lldb,ninja     # Install both compilers with specific tools
    $0 --dry-run                               # Preview installation without executing

SUPPORTED PLATFORMS:
    - Linux (Ubuntu/Debian with apt, CentOS/RHEL with yum/dnf, Arch with pacman)
    - macOS (with Homebrew)

SUPPORTED COMPILERS:
    - GCC (GNU Compiler Collection)
    - Clang (LLVM Clang)

SUPPORTED TOOLS:
    - GDB (GNU Debugger)
    - LLDB (LLVM Debugger)
    - Ninja (Build system)
    - Make (Build system)
    - CMake (Build system generator)
    - pkg-config (Package configuration tool)
EOF
}

# Function to log actions
log_action() {
    echo "[INFO] $1"
}

log_verbose() {
    if [ "$VERBOSE" = true ]; then
        echo "[VERBOSE] $1"
    fi
}

log_error() {
    echo "[ERROR] $1" >&2
}

# Function to execute or simulate command
execute_cmd() {
    if [ "$DRY_RUN" = true ]; then
        echo "[DRY-RUN] Would execute: $*"
    else
        log_verbose "Executing: $*"
        "$@"
    fi
}

# Function to check if command exists
check_command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--compilers)
            COMPILERS="$2"
            shift 2
            ;;
        -v|--version)
            VERSION="$2"
            shift 2
            ;;
        --tools)
            TOOLS="$2"
            shift 2
            ;;
        --no-cmake)
            INSTALL_CMAKE=false
            shift
            ;;
        --no-pkg-config)
            INSTALL_PKG_CONFIG=false
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

echo "==== C++ Development Environment Setup Script ===="
echo "Compilers: $COMPILERS"
echo "Version: $VERSION"
echo "Tools: $TOOLS"
echo "CMake: $([ "$INSTALL_CMAKE" = true ] && echo "Yes" || echo "Skip")"
echo "pkg-config: $([ "$INSTALL_PKG_CONFIG" = true ] && echo "Yes" || echo "Skip")"
if [ "$DRY_RUN" = true ]; then
    echo "Mode: DRY RUN (simulation only)"
fi
echo "================================================="

OS=$(uname -s)
ARCH=$(uname -m)

# Convert comma-separated lists to arrays
IFS=',' read -ra COMPILER_ARRAY <<< "$COMPILERS"
IFS=',' read -ra TOOLS_ARRAY <<< "$TOOLS"

# Function to detect package manager on Linux
detect_package_manager() {
    if check_command_exists apt-get; then
        echo "apt"
    elif check_command_exists yum; then
        echo "yum"
    elif check_command_exists dnf; then
        echo "dnf"
    elif check_command_exists pacman; then
        echo "pacman"
    else
        echo "unknown"
    fi
}

# Function to install packages safely
safe_install_packages() {
    local packages=("$@")
    local failed_packages=()
    
    for package in "${packages[@]}"; do
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY-RUN] Would install package: $package"
        else
            log_verbose "Installing package: $package"
            case "$PKG_MANAGER" in
                apt)
                    if ! execute_cmd sudo apt-get install -y "$package"; then
                        failed_packages+=("$package")
                    fi
                    ;;
                yum|dnf)
                    if ! execute_cmd sudo "$PKG_MANAGER" install -y "$package"; then
                        failed_packages+=("$package")
                    fi
                    ;;
                pacman)
                    if ! execute_cmd sudo pacman -S --noconfirm "$package"; then
                        failed_packages+=("$package")
                    fi
                    ;;
                brew)
                    if ! execute_cmd brew install "$package"; then
                        failed_packages+=("$package")
                    fi
                    ;;
            esac
        fi
    done
    
    if [ ${#failed_packages[@]} -gt 0 ]; then
        log_error "Failed to install packages: ${failed_packages[*]}"
    fi
}

# Function to install GCC
install_gcc() {
    log_action "Installing GCC compiler..."
    
    local packages=()
    
    case "$OS" in
        Linux)
            case "$PKG_MANAGER" in
                apt)
                    if [ "$VERSION" = "latest" ]; then
                        packages+=("build-essential" "gcc" "g++" "libc6-dev")
                    else
                        packages+=("gcc-$VERSION" "g++-$VERSION")
                        # Set up alternatives
                        if [ "$DRY_RUN" = false ]; then
                            execute_cmd sudo update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-$VERSION" 100 || true
                            execute_cmd sudo update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-$VERSION" 100 || true
                        fi
                    fi
                    ;;
                yum|dnf)
                    packages+=("gcc" "gcc-c++" "glibc-devel")
                    ;;
                pacman)
                    packages+=("gcc" "glibc")
                    ;;
            esac
            ;;
        Darwin)
            log_action "GCC on macOS will be installed via Homebrew"
            if [ "$VERSION" = "latest" ]; then
                packages+=("gcc")
            else
                packages+=("gcc@$VERSION")
            fi
            ;;
    esac
    
    safe_install_packages "${packages[@]}"
}

# Function to install Clang
install_clang() {
    log_action "Installing Clang compiler..."
    
    local packages=()
    
    case "$OS" in
        Linux)
            case "$PKG_MANAGER" in
                apt)
                    if [ "$VERSION" = "latest" ]; then
                        packages+=("clang" "libc++-dev" "libc++abi-dev")
                    else
                        packages+=("clang-$VERSION" "libc++-$VERSION-dev" "libc++abi-$VERSION-dev")
                        # Set up alternatives
                        if [ "$DRY_RUN" = false ]; then
                            execute_cmd sudo update-alternatives --install /usr/bin/clang clang "/usr/bin/clang-$VERSION" 100 || true
                            execute_cmd sudo update-alternatives --install /usr/bin/clang++ clang++ "/usr/bin/clang++-$VERSION" 100 || true
                        fi
                    fi
                    ;;
                yum|dnf)
                    packages+=("clang" "libc++" "libc++abi")
                    ;;
                pacman)
                    packages+=("clang" "libc++")
                    ;;
            esac
            ;;
        Darwin)
            log_action "Clang is included with Xcode Command Line Tools on macOS"
            if ! check_command_exists clang; then
                execute_cmd xcode-select --install || true
            fi
            if [ "$VERSION" != "latest" ]; then
                packages+=("llvm@$VERSION")
            fi
            ;;
    esac
    
    if [ ${#packages[@]} -gt 0 ]; then
        safe_install_packages "${packages[@]}"
    fi
}

# Function to install debugging tools
install_debug_tools() {
    log_action "Installing debugging tools..."
    
    local packages=()
    
    for tool in "${TOOLS_ARRAY[@]}"; do
        case "$tool" in
            gdb)
                case "$OS" in
                    Linux)
                        packages+=("gdb")
                        ;;
                    Darwin)
                        log_action "Note: GDB on macOS requires code signing. Consider using LLDB instead."
                        packages+=("gdb")
                        ;;
                esac
                ;;
            lldb)
                case "$OS" in
                    Linux)
                        case "$PKG_MANAGER" in
                            apt)
                                packages+=("lldb")
                                ;;
                            yum|dnf)
                                packages+=("lldb")
                                ;;
                            pacman)
                                packages+=("lldb")
                                ;;
                        esac
                        ;;
                    Darwin)
                        log_action "LLDB is included with Xcode Command Line Tools on macOS"
                        ;;
                esac
                ;;
        esac
    done
    
    if [ ${#packages[@]} -gt 0 ]; then
        safe_install_packages "${packages[@]}"
    fi
}

# Function to install build tools
install_build_tools() {
    log_action "Installing build tools..."
    
    local packages=()
    
    for tool in "${TOOLS_ARRAY[@]}"; do
        case "$tool" in
            ninja)
                packages+=("ninja-build")
                ;;
            make)
                case "$OS" in
                    Linux)
                        packages+=("make")
                        ;;
                    Darwin)
                        # make is typically included with Xcode Command Line Tools
                        if ! check_command_exists make; then
                            packages+=("make")
                        fi
                        ;;
                esac
                ;;
        esac
    done
    
    if [ "$INSTALL_CMAKE" = true ]; then
        packages+=("cmake")
    fi
    
    if [ "$INSTALL_PKG_CONFIG" = true ]; then
        packages+=("pkg-config")
    fi
    
    if [ ${#packages[@]} -gt 0 ]; then
        safe_install_packages "${packages[@]}"
    fi
}

# Function to verify installation
verify_installation() {
    log_action "Verifying installation..."
    
    local verification_failed=false
    
    # Check compilers
    for compiler in "${COMPILER_ARRAY[@]}"; do
        case "$compiler" in
            gcc)
                if check_command_exists gcc && check_command_exists g++; then
                    local gcc_version=$(gcc --version | head -n1)
                    log_action "✓ GCC installed: $gcc_version"
                else
                    log_error "✗ GCC not found"
                    verification_failed=true
                fi
                ;;
            clang)
                if check_command_exists clang && check_command_exists clang++; then
                    local clang_version=$(clang --version | head -n1)
                    log_action "✓ Clang installed: $clang_version"
                else
                    log_error "✗ Clang not found"
                    verification_failed=true
                fi
                ;;
        esac
    done
    
    # Check tools
    for tool in "${TOOLS_ARRAY[@]}"; do
        if check_command_exists "$tool"; then
            local tool_version=$($tool --version 2>/dev/null | head -n1 || echo "Version unknown")
            log_action "✓ $tool installed: $tool_version"
        else
            log_error "✗ $tool not found"
            verification_failed=true
        fi
    done
    
    # Check CMake
    if [ "$INSTALL_CMAKE" = true ]; then
        if check_command_exists cmake; then
            local cmake_version=$(cmake --version | head -n1)
            log_action "✓ CMake installed: $cmake_version"
        else
            log_error "✗ CMake not found"
            verification_failed=true
        fi
    fi
    
    # Check pkg-config
    if [ "$INSTALL_PKG_CONFIG" = true ]; then
        if check_command_exists pkg-config; then
            local pkgconfig_version=$(pkg-config --version)
            log_action "✓ pkg-config installed: $pkgconfig_version"
        else
            log_error "✗ pkg-config not found"
            verification_failed=true
        fi
    fi
    
    if [ "$verification_failed" = false ]; then
        log_action "✓ All tools verified successfully!"
    else
        log_error "✗ Some tools failed verification"
        return 1
    fi
}

# Main installation logic
main() {
    log_action "Starting C++ development environment setup..."
    log_action "Target OS: $OS ($ARCH)"
    
    case "$OS" in
        Linux)
            PKG_MANAGER=$(detect_package_manager)
            if [ "$PKG_MANAGER" = "unknown" ]; then
                log_error "Unsupported Linux distribution - no supported package manager found"
                exit 1
            fi
            log_action "Detected package manager: $PKG_MANAGER"
            
            # Update package lists
            case "$PKG_MANAGER" in
                apt)
                    execute_cmd sudo apt-get update
                    ;;
                yum|dnf)
                    execute_cmd sudo "$PKG_MANAGER" check-update || true
                    ;;
                pacman)
                    execute_cmd sudo pacman -Sy
                    ;;
            esac
            ;;
        Darwin)
            PKG_MANAGER="brew"
            if ! check_command_exists brew; then
                log_error "Homebrew not found. Please install Homebrew first."
                log_error "Visit: https://brew.sh/"
                exit 1
            fi
            
            # Ensure Xcode Command Line Tools are installed
            if ! check_command_exists clang; then
                log_action "Installing Xcode Command Line Tools..."
                execute_cmd xcode-select --install || true
            fi
            
            execute_cmd brew update || true
            ;;
        *)
            log_error "Unsupported operating system: $OS"
            log_error "Supported systems: Linux, macOS"
            exit 1
            ;;
    esac
    
    # Install compilers
    for compiler in "${COMPILER_ARRAY[@]}"; do
        case "$compiler" in
            gcc)
                install_gcc
                ;;
            clang)
                install_clang
                ;;
            msvc)
                log_error "MSVC is only supported on Windows. Use cpp.bat script on Windows."
                ;;
            *)
                log_error "Unknown compiler: $compiler"
                ;;
        esac
    done
    
    # Install debugging tools
    install_debug_tools
    
    # Install build tools
    install_build_tools
    
    if [ "$DRY_RUN" = false ]; then
        log_action "Installation completed!"
        verify_installation
        
        log_action "Quick test commands:"
        for compiler in "${COMPILER_ARRAY[@]}"; do
            case "$compiler" in
                gcc)
                    echo "  gcc --version && g++ --version"
                    ;;
                clang)
                    echo "  clang --version && clang++ --version"
                    ;;
            esac
        done
        echo "  cmake --version"
    else
        log_action "Dry run completed - no changes were made"
    fi
}

# Execute main function
main

echo "==== C++ development environment setup finished ====="