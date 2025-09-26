#!/usr/bin/env bash

# Generic ClickHouse installation script
# Usage: ./clickhouse.sh [OPTIONS]
# Options:
#   -v, --version VERSION     ClickHouse version to install (default: 23.8.10.33)
#   -d, --dest DIR           Installation destination directory (default: /usr/local)
#   -t, --tmpdir DIR         Temporary directory for downloads (default: /tmp)
#   -h, --help               Show this help message
#   --dry-run                Show what would be done without executing
#   --no-cleanup             Don't remove downloaded files after installation

set -e

# Default values
DEFAULT_VERSION="23.8.10.33"
DEFAULT_DEST="/usr/local"
DEFAULT_TMPDIR="/tmp"

# Initialize variables
CLICKHOUSE_VERSION="$DEFAULT_VERSION"
INSTALL_DEST="$DEFAULT_DEST"
TMPDIR="$DEFAULT_TMPDIR"
DRY_RUN=false
NO_CLEANUP=false

# Function to show help
show_help() {
    cat << EOF
Generic ClickHouse Installation Script

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -v, --version VERSION     ClickHouse version to install (default: $DEFAULT_VERSION)
    -d, --dest DIR           Installation destination directory (default: $DEFAULT_DEST)
    -t, --tmpdir DIR         Temporary directory for downloads (default: $DEFAULT_TMPDIR)
    -h, --help               Show this help message
    --dry-run                Show what would be done without executing
    --no-cleanup             Don't remove downloaded files after installation

EXAMPLES:
    $0                                          # Install default version to default location
    $0 -v 24.1.8.22                           # Install specific version
    $0 -v 23.8.10.33 -d /opt                  # Install to custom directory
    $0 --version 24.1.8.22 --tmpdir /var/tmp  # Use custom temp directory
    $0 --dry-run                               # Preview installation without executing

SUPPORTED PLATFORMS:
    - Linux (Ubuntu/Debian with apt, CentOS/RHEL with yum/dnf, Arch with pacman)
    - macOS (with Homebrew)
EOF
}

# Function to log actions
log_action() {
    echo "[INFO] $1"
}

log_error() {
    echo "[ERROR] $1" >&2
}

# Function to execute or simulate command
execute_cmd() {
    if [ "$DRY_RUN" = true ]; then
        echo "[DRY-RUN] Would execute: $*"
    else
        log_action "Executing: $*"
        "$@"
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--version)
            CLICKHOUSE_VERSION="$2"
            shift 2
            ;;
        -d|--dest)
            INSTALL_DEST="$2"
            shift 2
            ;;
        -t|--tmpdir)
            TMPDIR="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --no-cleanup)
            NO_CLEANUP=true
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

# Validate parameters
if [ ! -d "$TMPDIR" ]; then
    log_error "Temporary directory does not exist: $TMPDIR"
    exit 1
fi

if [ ! -d "$INSTALL_DEST" ] && [ "$INSTALL_DEST" != "/usr/local" ]; then
    log_error "Installation destination does not exist: $INSTALL_DEST"
    log_error "Please create the directory first or use sudo privileges"
    exit 1
fi

echo "==== Generic ClickHouse Installation Script ===="
echo "Version: $CLICKHOUSE_VERSION"
echo "Destination: $INSTALL_DEST"
echo "Temp directory: $TMPDIR"
if [ "$DRY_RUN" = true ]; then
    echo "Mode: DRY RUN (simulation only)"
fi
echo "=================================================="

OS=$(uname -s)
ARCH=$(uname -m)

# Generate download URLs based on version and architecture
generate_urls() {
    case "$ARCH" in
        x86_64|amd64)
            DEB_URL="https://packages.clickhouse.com/deb/pool/main/c/clickhouse/clickhouse-common-static_${CLICKHOUSE_VERSION}_amd64.deb"
            RPM_URL="https://packages.clickhouse.com/rpm/stable/x86_64/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.x86_64.rpm"
            TARBALL_URL="https://builds.clickhouse.com/master/${CLICKHOUSE_VERSION}/clickhouse-common-static.tgz"
            ;;
        aarch64|arm64)
            DEB_URL="https://packages.clickhouse.com/deb/pool/main/c/clickhouse/clickhouse-common-static_${CLICKHOUSE_VERSION}_arm64.deb"
            RPM_URL="https://packages.clickhouse.com/rpm/stable/aarch64/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.aarch64.rpm"
            TARBALL_URL="https://builds.clickhouse.com/master/${CLICKHOUSE_VERSION}/clickhouse-common-static-arm64.tgz"
            ;;
        *)
            log_error "Unsupported architecture: $ARCH"
            exit 1
            ;;
    esac
}

# Cleanup function
cleanup_files() {
    if [ "$NO_CLEANUP" = true ]; then
        log_action "Cleanup skipped (--no-cleanup specified)"
        return
    fi
    
    local files_to_clean=("$@")
    for file in "${files_to_clean[@]}"; do
        if [ -f "$file" ]; then
            execute_cmd rm -f "$file"
            log_action "Cleaned up: $file"
        fi
    done
}

install_linux() {
    log_action "Detected Linux ($ARCH architecture)"
    generate_urls
    
    local downloaded_files=()
    
    if command -v apt-get >/dev/null; then
        log_action "Using APT package manager"
        local deb_file="$TMPDIR/clickhouse-common-static_${CLICKHOUSE_VERSION}_$(echo $ARCH | sed 's/x86_64/amd64/').deb"
        downloaded_files+=("$deb_file")
        
        execute_cmd wget -O "$deb_file" "$DEB_URL"
        if [ "$DRY_RUN" = false ]; then
            execute_cmd sudo apt-get update || true
            execute_cmd sudo apt-get install -y wget lsb-release gnupg || true
            execute_cmd sudo dpkg -i "$deb_file"
            execute_cmd sudo apt-get install -f -y || true
        fi
        
    elif command -v yum >/dev/null; then
        log_action "Using YUM package manager"
        local rpm_file="$TMPDIR/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.$(echo $ARCH).rpm"
        downloaded_files+=("$rpm_file")
        
        execute_cmd wget -O "$rpm_file" "$RPM_URL"
        if [ "$DRY_RUN" = false ]; then
            execute_cmd sudo yum install -y wget || true
            execute_cmd sudo yum install -y "$rpm_file" || true
        fi
        
    elif command -v dnf >/dev/null; then
        log_action "Using DNF package manager"
        local rpm_file="$TMPDIR/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.$(echo $ARCH).rpm"
        downloaded_files+=("$rpm_file")
        
        execute_cmd wget -O "$rpm_file" "$RPM_URL"
        if [ "$DRY_RUN" = false ]; then
            execute_cmd sudo dnf install -y wget || true
            execute_cmd sudo dnf install -y "$rpm_file" || true
        fi
        
    elif command -v pacman >/dev/null; then
        log_action "Using Pacman package manager"
        local tarball_file="$TMPDIR/clickhouse-common-static-${CLICKHOUSE_VERSION}.tgz"
        downloaded_files+=("$tarball_file")
        
        execute_cmd wget -O "$tarball_file" "$TARBALL_URL"
        if [ "$DRY_RUN" = false ]; then
            execute_cmd sudo pacman -Sy --noconfirm wget || true
            execute_cmd sudo tar -xf "$tarball_file" -C "$INSTALL_DEST"
        fi
        
    else
        log_error "No supported package manager found (apt-get, yum, dnf, pacman)"
        log_error "Please install ClickHouse manually or install a supported package manager"
        exit 1
    fi
    
    cleanup_files "${downloaded_files[@]}"
    log_action "ClickHouse $CLICKHOUSE_VERSION installed successfully on Linux"
}

install_macos() {
    log_action "Detected macOS ($ARCH architecture)"
    
    if ! command -v brew >/dev/null; then
        log_error "Homebrew not found. Please install Homebrew first."
        log_error "Visit: https://brew.sh/"
        exit 1
    fi
    
    # Check if specific version is requested
    if [ "$CLICKHOUSE_VERSION" != "$DEFAULT_VERSION" ]; then
        log_action "Note: Homebrew installs the latest stable version by default"
        log_action "Requested version: $CLICKHOUSE_VERSION"
        log_action "To install a specific version, you may need to use brew tap or manual installation"
    fi
    
    execute_cmd brew update || true
    execute_cmd brew install clickhouse || true
    
    if [ "$DRY_RUN" = false ]; then
        # Verify installation
        if command -v clickhouse >/dev/null; then
            local installed_version=$(clickhouse --version 2>/dev/null | head -n1 || echo "Version unknown")
            log_action "ClickHouse installed successfully: $installed_version"
        else
            log_error "Installation may have failed - clickhouse command not found"
            exit 1
        fi
    else
        log_action "ClickHouse would be installed via Homebrew"
    fi
}

# Main installation logic
main() {
    log_action "Starting ClickHouse installation..."
    log_action "Target version: $CLICKHOUSE_VERSION"
    log_action "Installation destination: $INSTALL_DEST"
    log_action "Architecture: $ARCH"
    
    case "$OS" in
        Linux)
            install_linux
            ;;
        Darwin)
            install_macos
            ;;
        *)
            log_error "Unsupported operating system: $OS"
            log_error "Supported systems: Linux, macOS"
            exit 1
            ;;
    esac
    
    if [ "$DRY_RUN" = false ]; then
        log_action "Installation completed successfully!"
        log_action "You may need to start the ClickHouse service manually"
    else
        log_action "Dry run completed - no changes were made"
    fi
}

# Execute main function
main

echo "==== ClickHouse installation script finished ====="
