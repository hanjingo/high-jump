#!/bin/bash

set -e


# Default database, user, and password
DBNAME="testdb"
DBUSER="testuser"
DBPASS="testpass"


# Install PostgreSQL on Ubuntu/Debian
install_ubuntu() {
	sudo apt update
	sudo apt install -y postgresql postgresql-contrib
	sudo systemctl enable postgresql
	sudo systemctl start postgresql
}


# Install PostgreSQL on CentOS/RHEL/Fedora
install_centos() {
	sudo yum install -y epel-release
	sudo yum install -y postgresql-server postgresql-contrib
	sudo postgresql-setup initdb
	sudo systemctl enable postgresql
	sudo systemctl start postgresql
}


# Install PostgreSQL on macOS
install_macos() {
	if ! command -v brew >/dev/null 2>&1; then
		echo "Homebrew is not installed. Please install Homebrew first."
		exit 1
	fi
	brew update
	brew install postgresql
	brew services start postgresql
}


# Initialize PostgreSQL: set password, create database and user
init_postgres() {
	sudo -u postgres psql -c "ALTER USER postgres WITH PASSWORD 'postgres';"
	sudo -u postgres psql <<EOF
CREATE DATABASE $DBNAME;
CREATE USER $DBUSER WITH ENCRYPTED PASSWORD '$DBPASS';
GRANT ALL PRIVILEGES ON DATABASE $DBNAME TO $DBUSER;
EOF
}


# Detect OS and run installation/configuration
case "$(uname -s)" in
	Linux)
		if [ -f /etc/os-release ]; then
			. /etc/os-release
			case "$ID" in
				ubuntu|debian)
					install_ubuntu
					;;
				centos|rhel|fedora)
					install_centos
					;;
				*)
					echo "Unsupported Linux distribution: $ID"
					exit 1
					;;
			esac
		else
			echo "Unrecognized Linux system."
			exit 1
		fi
		init_postgres
		;;
	Darwin)
		install_macos
		# On macOS, use current user for psql
		psql postgres <<EOF
ALTER USER postgres WITH PASSWORD 'postgres';
CREATE DATABASE $DBNAME;
CREATE USER $DBUSER WITH ENCRYPTED PASSWORD '$DBPASS';
GRANT ALL PRIVILEGES ON DATABASE $DBNAME TO $DBUSER;
EOF
		;;
	*)
		echo "Unsupported operating system: $(uname -s)"
		exit 1
		;;
esac

echo "PostgreSQL installation and configuration completed!"
echo "Database: $DBNAME, User: $DBUSER, Password: $DBPASS"
