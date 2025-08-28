#!/usr/bin/env bash

set -e

echo "==== ClickHouse install script ===="

OS=$(uname -s)
TMPDIR="/tmp"
CLICKHOUSE_VERSION="23.8.10.33"
CLICKHOUSE_DIR="clickhouse-$CLICKHOUSE_VERSION"
CLICKHOUSE_TARBALL_URL="https://packages.clickhouse.com/deb/pool/main/c/clickhouse/clickhouse-common-static_${CLICKHOUSE_VERSION}_amd64.deb"
CLICKHOUSE_TARBALL="$TMPDIR/clickhouse-common-static_${CLICKHOUSE_VERSION}_amd64.deb"

install_linux() {
    echo "Detected Linux"
    if command -v apt-get >/dev/null; then
        sudo apt-get update || true
        sudo apt-get install -y wget lsb-release gnupg || true
        wget -O $CLICKHOUSE_TARBALL $CLICKHOUSE_TARBALL_URL
        sudo dpkg -i $CLICKHOUSE_TARBALL
        sudo apt-get install -f -y || true
    elif command -v yum >/dev/null; then
        sudo yum install -y wget || true
        # RPM package URL for ClickHouse
        RPM_URL="https://packages.clickhouse.com/rpm/stable/x86_64/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.x86_64.rpm"
        RPM_FILE="$TMPDIR/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.x86_64.rpm"
        wget -O $RPM_FILE $RPM_URL
        sudo yum install -y $RPM_FILE || true
        rm -f "$RPM_FILE"
    elif command -v dnf >/dev/null; then
        sudo dnf install -y wget || true
        RPM_URL="https://packages.clickhouse.com/rpm/stable/x86_64/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.x86_64.rpm"
        RPM_FILE="$TMPDIR/clickhouse-common-static-${CLICKHOUSE_VERSION}-1.x86_64.rpm"
        wget -O $RPM_FILE $RPM_URL
        sudo dnf install -y $RPM_FILE || true
        rm -f "$RPM_FILE"
    elif command -v pacman >/dev/null; then
        sudo pacman -Sy --noconfirm wget || true
        # Use tarball for manual install
        TARBALL_URL="https://builds.clickhouse.com/master/${CLICKHOUSE_VERSION}/clickhouse-common-static.tgz"
        TARBALL_FILE="$TMPDIR/clickhouse-common-static.tgz"
        wget -O $TARBALL_FILE $TARBALL_URL
        tar -xf $TARBALL_FILE -C /usr/local
        rm -f "$TARBALL_FILE"
    else
        echo "Unknown Linux distribution. Please install ClickHouse manually."
        exit 1
    fi
    rm -f "$CLICKHOUSE_TARBALL"
    echo "ClickHouse installed."
}

install_macos() {
    echo "Detected macOS"
    if ! command -v brew >/dev/null; then
        echo "Homebrew not found. Please install Homebrew first."
        exit 1
    fi
    brew update || true
    brew install clickhouse || true
    echo "ClickHouse installed."
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

echo "==== ClickHouse install finished ===="
