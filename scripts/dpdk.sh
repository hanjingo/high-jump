#!/usr/bin/env bash

set -e

echo "==== DPDK install script ===="

OS=$(uname -s)
DPDK_VERSION="23.11"
DPDK_DIR="dpdk-$DPDK_VERSION"

TMPDIR="/tmp"
DPDK_TARBALL_URL="https://fast.dpdk.org/rel/$DPDK_DIR.tar.xz"
DPDK_TARBALL="$TMPDIR/$DPDK_DIR.tar.xz"


install_linux() {
    echo "Detected Linux"
    if command -v apt-get >/dev/null; then
        sudo apt-get update || true
        sudo apt-get install -y build-essential meson ninja-build python3-pyelftools libnuma-dev wget || true
    elif command -v yum >/dev/null; then
        sudo yum install -y epel-release || true
        sudo yum install -y gcc gcc-c++ make meson ninja-build python3-pyelftools numactl-devel wget || true
    elif command -v dnf >/dev/null; then
        sudo dnf install -y gcc gcc-c++ make meson ninja-build python3-pyelftools numactl-devel wget || true
    elif command -v pacman >/dev/null; then
        sudo pacman -Sy --noconfirm base-devel meson ninja python-pyelftools numactl wget || true
    else
        echo "Unknown Linux distribution. Please install build-essential, meson, ninja, python3-pyelftools, libnuma-dev, wget manually."
    fi
    wget -O $DPDK_TARBALL $DPDK_TARBALL_URL
    tar -xf $DPDK_TARBALL
    cd $DPDK_DIR
    meson setup build
    ninja -C build
    sudo ninja -C build install
    sudo ldconfig || true
    echo "DPDK installed to /usr/local"
    rm -f "$DPDK_TARBALL"
}

install_macos() {
    echo "Detected macOS"
    if ! command -v brew >/dev/null; then
        echo "Homebrew not found. Please install Homebrew first."
        exit 1
    fi
    brew update || true
    brew install meson ninja python@3.11 || true
    wget -O $DPDK_TARBALL $DPDK_TARBALL_URL
    tar -xf $DPDK_TARBALL
    cd $DPDK_DIR
    meson setup build
    ninja -C build
    sudo ninja -C build install
    echo "DPDK installed to /usr/local"
    rm -f "$DPDK_TARBALL"
}

case "$OS" in
    Linux)
        install_linux
        ;;
    Darwin)
        install_macos
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

echo "==== DPDK install finished ===="