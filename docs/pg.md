# PostgreSQL Installation Script Documentation

A generic PostgreSQL installation and configuration script for cross-platform deployment.

## Overview

The `pg.sh` script provides an automated way to install and configure PostgreSQL on various operating systems. It supports multiple PostgreSQL versions, custom configurations, and flexible deployment options.

## Features

- **Cross-platform support**: Linux (Ubuntu/Debian, CentOS/RHEL/Fedora) and macOS
- **Version flexibility**: Install specific PostgreSQL versions or latest stable
- **Custom configuration**: Configurable database name, user, password, and port
- **Installation control**: Options to skip initialization or service startup
- **Dry-run mode**: Preview installation steps without making changes
- **Comprehensive logging**: Detailed output for installation tracking

## Usage

### Basic Syntax

```bash
./scripts/pg.sh [OPTIONS]
```

### Quick Start

```bash
# Install PostgreSQL with default settings
./scripts/pg.sh

# Install specific version with custom user
./scripts/pg.sh -v 15 -u myuser -p mypassword

# Preview installation without executing
./scripts/pg.sh --dry-run
```

## Options

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--version VERSION` | `-v` | PostgreSQL version to install | `latest` |
| `--dest DIR` | `-d` | Installation destination directory | system default |
| `--user USERNAME` | `-u` | Database user to create | `pguser` |
| `--password PASSWORD` | `-p` | Password for database user | `pgpass` |
| `--database DBNAME` | | Database name to create | `postgres` |
| `--port PORT` | | PostgreSQL port | `5432` |
| `--help` | `-h` | Show help message | |
| `--dry-run` | | Show what would be done without executing | |
| `--no-init` | | Skip database initialization | |
| `--no-start` | | Skip starting PostgreSQL service | |

## Examples

### Version-specific Installation

```bash
# Install PostgreSQL 15
./scripts/pg.sh -v 15

# Install PostgreSQL 14 with custom configuration
./scripts/pg.sh -v 14 -u appuser -p apppass --database myapp
```

### Custom Configuration

```bash
# Custom user and database
./scripts/pg.sh -u myuser -p mypass --database mydb

# Custom port and skip auto-start
./scripts/pg.sh --port 5433 --no-start

# Install only, skip initialization
./scripts/pg.sh --no-init
```

### Development Workflow

```bash
# Preview installation
./scripts/pg.sh --dry-run

# Install for development
./scripts/pg.sh -u devuser -p devpass --database devdb --port 5433

# Production installation with specific version
./scripts/pg.sh -v 15 -u produser -p $(cat /secure/pgpass) --database proddb
```

## Platform Support

### Linux

#### Ubuntu/Debian
- Uses `apt` package manager
- Supports specific versions via PostgreSQL official repository
- Requires `sudo` privileges

#### CentOS/RHEL/Fedora/Rocky/AlmaLinux
- Uses `yum` or `dnf` package manager
- Supports PostgreSQL official repository for specific versions
- Requires `sudo` privileges

### macOS
- Uses Homebrew package manager
- Homebrew must be installed first
- Limited version support (depends on Homebrew formulas)

## Installation Process

### 1. Package Installation
- Installs PostgreSQL server and contrib packages
- Adds official repositories for specific versions when needed
- Handles platform-specific package manager differences

### 2. Service Configuration
- Enables PostgreSQL service (unless `--no-start` specified)
- Starts PostgreSQL service
- Configures system service management

### 3. Database Initialization
- Sets up initial database cluster (Linux only, unless `--no-init` specified)
- Configures postgres superuser password
- Creates custom database and user with specified credentials
- Grants appropriate privileges

## Security Considerations

### Password Handling
```bash
# Use environment variables for sensitive data
export PG_PASS="your-secure-password"
./scripts/pg.sh -p "$PG_PASS"

# Read from secure file
./scripts/pg.sh -p "$(cat /secure/postgres.pass)"
```

### User Privileges
- Created users have `CREATEDB` privilege
- Full privileges granted on specified database
- Consider principle of least privilege for production

## Troubleshooting

### Common Issues

**Permission denied errors**
```bash
# Ensure script is executable
chmod +x scripts/pg.sh

# Run with proper privileges on Linux
sudo ./scripts/pg.sh
```

**Service not starting**
```bash
# Check service status (Linux)
sudo systemctl status postgresql

# Check logs (Linux)
sudo journalctl -u postgresql

# Check Homebrew services (macOS)
brew services list | grep postgresql
```

**Connection issues**
```bash
# Test connection
psql -h localhost -p 5432 -U pguser -d postgres

# Check port availability
netstat -tulpn | grep :5432
```

### Verification

```bash
# Check PostgreSQL version
psql --version

# Test database connection
psql -h localhost -U pguser -d postgres -c "SELECT version();"

# List databases
psql -h localhost -U pguser -c "\\l"
```

## Integration Examples

### Docker Environment

```bash
# Install PostgreSQL for containerized development
./scripts/pg.sh -u dockeruser -p dockerpass --database containerdb --port 5434
```

### CI/CD Pipeline

```bash
# Automated testing setup
./scripts/pg.sh --dry-run  # Verify configuration
./scripts/pg.sh -u testuser -p testpass --database testdb --no-start
sudo systemctl start postgresql
```

### Development Environment

```bash
# Local development setup
./scripts/pg.sh -v 15 -u devuser -p devpass --database myapp_dev --port 5432
```

## Advanced Usage

### Custom Installation Path

```bash
# Install to custom directory (when supported)
./scripts/pg.sh -d /opt/postgresql
```

### Multiple Versions

```bash
# Install different versions for testing
./scripts/pg.sh -v 14 --port 5434 --no-start
./scripts/pg.sh -v 15 --port 5435 --no-start
```

## Requirements

### System Requirements
- Linux: Ubuntu 18.04+, Debian 10+, CentOS 7+, RHEL 7+, Fedora 30+
- macOS: 10.14+ with Homebrew installed
- Sufficient disk space (varies by version, typically 50MB+ for base installation)

### Permissions
- Linux: `sudo` privileges required for system package installation
- macOS: Homebrew write permissions

### Dependencies
- `wget` or `curl` for downloading packages (Linux)
- Internet connection for package downloads

## Notes

- The script automatically detects the operating system and uses appropriate package managers
- On Linux, PostgreSQL data directory location varies by distribution
- macOS installations via Homebrew may have different default configurations
- Always test in a non-production environment first
- Consider firewall and network security settings for production deployments

## Related Documentation

- [PostgreSQL Official Documentation](https://www.postgresql.org/docs/)
- [Homebrew PostgreSQL Formula](https://formulae.brew.sh/formula/postgresql)
- [PostgreSQL APT Repository](https://wiki.postgresql.org/wiki/Apt)
- [PostgreSQL YUM Repository](https://yum.postgresql.org/)