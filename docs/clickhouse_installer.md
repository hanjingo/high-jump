# Generic ClickHouse Installation Script

A configurable, cross-platform script for installing ClickHouse with customizable options.

## Features

- **Cross-Platform**: Supports Linux (Ubuntu/Debian, CentOS/RHEL, Arch) and macOS
- **Configurable Version**: Install any specific ClickHouse version
- **Custom Installation Path**: Choose installation destination
- **Architecture Support**: Supports both x86_64/amd64 and aarch64/arm64 architectures
- **Dry Run Mode**: Preview installation without making changes
- **Multiple Package Managers**: Automatic detection and use of apt, yum, dnf, pacman, or Homebrew
- **Cleanup Options**: Automatic cleanup with option to preserve downloaded files

## Usage

### Basic Installation
```bash
# Install default version (23.8.10.33) to default location (/usr/local)
./clickhouse.sh
```

### Custom Version
```bash
# Install specific version
./clickhouse.sh --version 24.1.8.22
./clickhouse.sh -v 24.1.8.22
```

### Custom Installation Directory
```bash
# Install to custom directory (requires appropriate permissions)
./clickhouse.sh --dest /opt
./clickhouse.sh -d /opt/clickhouse
```

### Custom Temporary Directory
```bash
# Use different temp directory for downloads
./clickhouse.sh --tmpdir /var/tmp
./clickhouse.sh -t /home/user/tmp
```

### Dry Run (Preview)
```bash
# See what would be installed without making changes
./clickhouse.sh --dry-run
./clickhouse.sh --dry-run --version 24.1.8.22 --dest /opt
```

### Advanced Options
```bash
# Don't cleanup downloaded files (for debugging or reuse)
./clickhouse.sh --no-cleanup

# Combine multiple options
./clickhouse.sh -v 24.1.8.22 -d /opt -t /var/tmp --dry-run
```

## Supported Platforms

### Linux
- **Ubuntu/Debian**: Uses APT package manager (.deb packages)
- **CentOS/RHEL**: Uses YUM/DNF package manager (.rpm packages)  
- **Arch Linux**: Uses Pacman with tarball extraction
- **Generic**: Falls back to tarball installation for unsupported distributions

### macOS
- **Homebrew**: Automatic installation via `brew install clickhouse`
- **Note**: Homebrew typically installs the latest stable version regardless of `--version` parameter

## Architecture Support

The script automatically detects system architecture and downloads appropriate packages:
- **x86_64/amd64**: Standard Intel/AMD 64-bit
- **aarch64/arm64**: ARM 64-bit (Apple Silicon, ARM servers)

## Examples

### Development Environment Setup
```bash
# Install latest version for development
./clickhouse.sh --version 24.1.8.22

# Preview installation first
./clickhouse.sh --dry-run --version 24.1.8.22
```

### Production Server Installation
```bash
# Install specific stable version to /opt
./clickhouse.sh --version 23.8.10.33 --dest /opt

# Use custom temp directory and preserve downloads
./clickhouse.sh -v 23.8.10.33 -d /opt -t /var/tmp --no-cleanup
```

### CI/CD Pipeline
```bash
# Non-interactive installation with specific version
./clickhouse.sh --version 24.1.8.22 2>&1 | tee installation.log
```

## Options Reference

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--version VERSION` | `-v` | ClickHouse version to install | `23.8.10.33` |
| `--dest DIR` | `-d` | Installation destination directory | `/usr/local` |
| `--tmpdir DIR` | `-t` | Temporary directory for downloads | `/tmp` |
| `--help` | `-h` | Show help message | - |
| `--dry-run` | - | Preview without executing | `false` |
| `--no-cleanup` | - | Don't remove downloaded files | `false` |

## Troubleshooting

### Permission Issues
- For system-wide installation, you may need `sudo` privileges
- Consider using `--dest` to install to a user-writable directory

### Package Manager Issues
```bash
# Check if required tools are installed
which wget curl
which apt-get yum dnf pacman brew

# On macOS, install Homebrew first if missing
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Version Availability
- Check ClickHouse releases: https://github.com/ClickHouse/ClickHouse/releases
- Some very old or very new versions might not be available for all platforms

### Network Issues
```bash
# Test connectivity to ClickHouse package repositories
curl -I https://packages.clickhouse.com/
wget --spider https://builds.clickhouse.com/
```

## Integration Examples

### In Project Scripts
```bash
#!/bin/bash
# Project setup script
echo "Installing ClickHouse for development..."
./scripts/clickhouse.sh --version 23.8.10.33 --dry-run
read -p "Proceed with installation? (y/N) " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    ./scripts/clickhouse.sh --version 23.8.10.33
fi
```

### Docker Integration
```dockerfile
# Copy script and run during container build
COPY scripts/clickhouse.sh /tmp/
RUN chmod +x /tmp/clickhouse.sh && \
    /tmp/clickhouse.sh --version 24.1.8.22 && \
    rm /tmp/clickhouse.sh
```

## Exit Codes

- `0`: Success
- `1`: General error (invalid arguments, unsupported platform, installation failure)

## Requirements

- **Linux**: wget, appropriate package manager (apt-get, yum, dnf, or pacman)
- **macOS**: Homebrew
- **All platforms**: Internet connectivity, appropriate permissions for installation directory