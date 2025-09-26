#!/usr/bin/env bash

# Generic PostgreSQL installation script
# Usage: ./pg.sh [OPTIONS]
# Options:
#   -v, --version VERSION     PostgreSQL version to install (default: latest)
#   -d, --dest DIR           Installation destination directory (default: system default)
#   -u, --user USERNAME      Database user to create (default: pguser)
#   -p, --password PASSWORD  Password for database user (default: pgpass)
#   --database DBNAME        Database name to create (default: postgres)
#   --port PORT              PostgreSQL port (default: 5432)
#   -h, --help               Show this help message
#   --dry-run                Show what would be done without executing
#   --no-init                Skip database initialization
#   --no-start               Skip starting PostgreSQL service

set -e

# Default values
DEFAULT_VERSION="latest"
DEFAULT_DEST=""
DEFAULT_USER="pguser"
DEFAULT_PASSWORD="pgpass"
DEFAULT_DATABASE="postgres"
DEFAULT_PORT="5432"

# Initialize variables
PG_VERSION="$DEFAULT_VERSION"
INSTALL_DEST="$DEFAULT_DEST"
DB_USER="$DEFAULT_USER"
DB_PASS="$DEFAULT_PASSWORD"
DB_NAME="$DEFAULT_DATABASE"
DB_PORT="$DEFAULT_PORT"
DRY_RUN=false
NO_INIT=false
NO_START=false

# Function to show help
show_help() {
    cat << EOF
Generic PostgreSQL Installation Script

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -v, --version VERSION     PostgreSQL version to install (default: $DEFAULT_VERSION)
    -d, --dest DIR           Installation destination directory (default: system default)
    -u, --user USERNAME      Database user to create (default: $DEFAULT_USER)
    -p, --password PASSWORD  Password for database user (default: $DEFAULT_PASSWORD)
    --database DBNAME        Database name to create (default: $DEFAULT_DATABASE)
    --port PORT              PostgreSQL port (default: $DEFAULT_PORT)
    -h, --help               Show this help message
    --dry-run                Show what would be done without executing
    --no-init                Skip database initialization
    --no-start               Skip starting PostgreSQL service

EXAMPLES:
    $0                                          # Install latest version with defaults
    $0 -v 15                                   # Install specific version
    $0 -u myuser -p mypass --database mydb    # Custom user and database
    $0 --port 5433 --no-start                 # Custom port, don't start service
    $0 --dry-run                               # Preview installation without executing

SUPPORTED PLATFORMS:
    - Linux (Ubuntu/Debian with apt, CentOS/RHEL with yum/dnf)
    - macOS (with Homebrew)

NOTE:
    This script requires sudo privileges on Linux systems.
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
            PG_VERSION="$2"
            shift 2
            ;;
        -d|--dest)
            INSTALL_DEST="$2"
            shift 2
            ;;
        -u|--user)
            DB_USER="$2"
            shift 2
            ;;
        -p|--password)
            DB_PASS="$2"
            shift 2
            ;;
        --database)
            DB_NAME="$2"
            shift 2
            ;;
        --port)
            DB_PORT="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --no-init)
            NO_INIT=true
            shift
            ;;
        --no-start)
            NO_START=true
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
if [ -n "$INSTALL_DEST" ] && [ ! -d "$INSTALL_DEST" ]; then
    log_error "Installation destination does not exist: $INSTALL_DEST"
    exit 1
fi

echo "==== Generic PostgreSQL Installation Script ===="
echo "Version: $PG_VERSION"
echo "Database User: $DB_USER"
echo "Database Name: $DB_NAME"
echo "Port: $DB_PORT"
if [ -n "$INSTALL_DEST" ]; then
    echo "Destination: $INSTALL_DEST"
fi
if [ "$DRY_RUN" = true ]; then
    echo "Mode: DRY RUN (simulation only)"
fi
echo "=================================================="

OS=$(uname -s)
ARCH=$(uname -m)


# Install PostgreSQL on Ubuntu/Debian
install_ubuntu() {
    log_action "Installing PostgreSQL on Ubuntu/Debian"
    
    if [ "$PG_VERSION" != "latest" ]; then
        # Add PostgreSQL official repository for specific versions
        execute_cmd wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
        execute_cmd echo "deb http://apt.postgresql.org/pub/repos/apt/ $(lsb_release -cs)-pgdg main" | sudo tee /etc/apt/sources.list.d/pgdg.list
        execute_cmd sudo apt update
        execute_cmd sudo apt install -y "postgresql-$PG_VERSION" "postgresql-contrib-$PG_VERSION"
    else
        execute_cmd sudo apt update
        execute_cmd sudo apt install -y postgresql postgresql-contrib
    fi
    
    if [ "$NO_START" = false ]; then
        execute_cmd sudo systemctl enable postgresql
        execute_cmd sudo systemctl start postgresql
    fi
}

# Install PostgreSQL on CentOS/RHEL/Fedora
install_centos() {
    log_action "Installing PostgreSQL on CentOS/RHEL/Fedora"
    
    if command -v dnf >/dev/null; then
        PKG_MGR="dnf"
    else
        PKG_MGR="yum"
    fi
    
    if [ "$PG_VERSION" != "latest" ]; then
        # Install PostgreSQL repository
        execute_cmd sudo $PKG_MGR install -y "https://download.postgresql.org/pub/repos/yum/reporpms/EL-$(rpm -E %{rhel})-x86_64/pgdg-redhat-repo-latest.noarch.rpm" || true
        execute_cmd sudo $PKG_MGR install -y "postgresql$PG_VERSION-server" "postgresql$PG_VERSION-contrib"
        
        if [ "$NO_INIT" = false ] && [ "$DRY_RUN" = false ]; then
            sudo "/usr/pgsql-$PG_VERSION/bin/postgresql-$PG_VERSION-setup" initdb || true
        fi
        
        if [ "$NO_START" = false ]; then
            execute_cmd sudo systemctl enable "postgresql-$PG_VERSION"
            execute_cmd sudo systemctl start "postgresql-$PG_VERSION"
        fi
    else
        execute_cmd sudo $PKG_MGR install -y postgresql-server postgresql-contrib
        
        if [ "$NO_INIT" = false ]; then
            execute_cmd sudo postgresql-setup initdb
        fi
        
        if [ "$NO_START" = false ]; then
            execute_cmd sudo systemctl enable postgresql
            execute_cmd sudo systemctl start postgresql
        fi
    fi
}

# Install PostgreSQL on macOS
install_macos() {
    log_action "Installing PostgreSQL on macOS"
    
    if ! command -v brew >/dev/null; then
        log_error "Homebrew not found. Please install Homebrew first."
        log_error "Visit: https://brew.sh/"
        exit 1
    fi
    
    execute_cmd brew update || true
    
    if [ "$PG_VERSION" != "latest" ]; then
        # Try to install specific version
        log_action "Note: Homebrew may not support all PostgreSQL versions"
        log_action "Attempting to install PostgreSQL $PG_VERSION"
        execute_cmd brew install "postgresql@$PG_VERSION" || execute_cmd brew install postgresql
    else
        execute_cmd brew install postgresql
    fi
    
    if [ "$NO_START" = false ]; then
        if [ "$PG_VERSION" != "latest" ]; then
            execute_cmd brew services start "postgresql@$PG_VERSION" || execute_cmd brew services start postgresql
        else
            execute_cmd brew services start postgresql
        fi
    fi
}


# Initialize PostgreSQL: set password, create database and user
init_postgres() {
    if [ "$NO_INIT" = true ]; then
        log_action "Skipping PostgreSQL initialization (--no-init specified)"
        return
    fi
    
    log_action "Initializing PostgreSQL database"
    
    case "$OS" in
        Linux)
            if [ "$DRY_RUN" = false ]; then
                # Set postgres user password
                sudo -u postgres psql -c "ALTER USER postgres WITH PASSWORD 'postgres';" || true
                
                # Create database and user
                sudo -u postgres psql <<EOF || true
CREATE DATABASE $DB_NAME;
CREATE USER $DB_USER WITH ENCRYPTED PASSWORD '$DB_PASS';
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
ALTER USER $DB_USER CREATEDB;
EOF
            else
                echo "[DRY-RUN] Would set postgres user password"
                echo "[DRY-RUN] Would create database: $DB_NAME"
                echo "[DRY-RUN] Would create user: $DB_USER with password: $DB_PASS"
                echo "[DRY-RUN] Would grant privileges to user"
            fi
            ;;
        Darwin)
            if [ "$DRY_RUN" = false ]; then
                # On macOS, use current user for psql
                psql postgres <<EOF || true
ALTER USER postgres WITH PASSWORD 'postgres';
CREATE DATABASE $DB_NAME;
CREATE USER $DB_USER WITH ENCRYPTED PASSWORD '$DB_PASS';
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
ALTER USER $DB_USER CREATEDB;
EOF
            else
                echo "[DRY-RUN] Would set postgres user password on macOS"
                echo "[DRY-RUN] Would create database: $DB_NAME"
                echo "[DRY-RUN] Would create user: $DB_USER with password: $DB_PASS"
                echo "[DRY-RUN] Would grant privileges to user"
            fi
            ;;
    esac
}

# Main installation logic
main() {
    log_action "Starting PostgreSQL installation..."
    log_action "Target version: $PG_VERSION"
    log_action "Architecture: $ARCH"
    
    case "$OS" in
        Linux)
            if [ -f /etc/os-release ]; then
                . /etc/os-release
                case "$ID" in
                    ubuntu|debian)
                        install_ubuntu
                        ;;
                    centos|rhel|fedora|rocky|almalinux)
                        install_centos
                        ;;
                    *)
                        log_error "Unsupported Linux distribution: $ID"
                        log_error "Supported distributions: Ubuntu, Debian, CentOS, RHEL, Fedora, Rocky Linux, AlmaLinux"
                        exit 1
                        ;;
                esac
            else
                log_error "Unrecognized Linux system - /etc/os-release not found"
                exit 1
            fi
            init_postgres
            ;;
        Darwin)
            install_macos
            init_postgres
            ;;
        *)
            log_error "Unsupported operating system: $OS"
            log_error "Supported systems: Linux, macOS"
            exit 1
            ;;
    esac
    
    if [ "$DRY_RUN" = false ]; then
        log_action "PostgreSQL installation completed successfully!"
        log_action "Database: $DB_NAME"
        log_action "User: $DB_USER"
        log_action "Port: $DB_PORT"
        
        if [ "$NO_START" = false ]; then
            log_action "PostgreSQL service has been started"
            log_action "You can connect using: psql -h localhost -p $DB_PORT -U $DB_USER -d $DB_NAME"
        else
            log_action "PostgreSQL service was not started (--no-start specified)"
            log_action "Start it manually with your system's service manager"
        fi
    else
        log_action "Dry run completed - no changes were made"
    fi
}

# Execute main function
main

echo "==== PostgreSQL installation script finished ====="
