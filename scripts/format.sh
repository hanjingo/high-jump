#!/bin/bash
# Cross-platform C++ code formatting script for Linux and macOS

set -euo pipefail

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Color output functions
red() { echo -e "\033[31m$*\033[0m"; }
green() { echo -e "\033[32m$*\033[0m"; }
yellow() { echo -e "\033[33m$*\033[0m"; }
blue() { echo -e "\033[34m$*\033[0m"; }

# Usage function
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Format C++ code using clang-format

Options:
    -c, --check     Only check formatting without making changes
    -h, --help      Show this help message
    -v, --verbose   Enable verbose output

Examples:
    $0              Format all C++ files
    $0 --check      Check formatting without changes
    $0 -c -v        Check formatting with verbose output
EOF
}

# Parse command line arguments
CHECK_ONLY=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--check)
            CHECK_ONLY=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

# Function to log verbose messages
log_verbose() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo "$@"
    fi
}

# Ensure we're in the project root
cd "$PROJECT_ROOT"

if [[ ! -f ".clang-format" ]]; then
    red "Error: Must be run from project root (directory containing .clang-format)"
    exit 1
fi

log_verbose "Project root: $PROJECT_ROOT"

# Check if clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
    red "Error: clang-format not found. Please install clang-format."
    echo "Installation instructions:"
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "  Ubuntu/Debian: sudo apt-get install clang-format"
        echo "  CentOS/RHEL:   sudo yum install clang-tools-extra"
        echo "  Fedora:        sudo dnf install clang-tools-extra"
        echo "  Arch:          sudo pacman -S clang"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  macOS:         brew install clang-format"
        echo "                 or install Xcode Command Line Tools"
    fi
    
    exit 1
fi

# Get clang-format version
CLANG_FORMAT_VERSION=$(clang-format --version)
log_verbose "Using: $CLANG_FORMAT_VERSION"

# Find all C++ files
log_verbose "Searching for C++ files in libcpp/ and tests/ directories..."

# Use find to locate C++ files
mapfile -t files < <(find libcpp tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" -o -name "*.cxx" \) 2>/dev/null || true)

if [[ ${#files[@]} -eq 0 ]]; then
    yellow "Warning: No C++ files found"
    exit 0
fi

echo "Found ${#files[@]} C++ files"

if [[ "$VERBOSE" == "true" ]]; then
    echo "Files to process:"
    printf '  %s\n' "${files[@]}"
fi

if [[ "$CHECK_ONLY" == "true" ]]; then
    blue "Checking code formatting..."
    has_errors=false
    error_files=()
    
    for file in "${files[@]}"; do
        log_verbose "Checking: $file"
        
        # Use a temporary file to capture clang-format output
        temp_file=$(mktemp)
        
        # Run clang-format and capture both stdout and stderr
        if ! clang-format --style=file --dry-run --Werror "$file" 2>"$temp_file"; then
            has_errors=true
            error_files+=("$file")
            
            red "Format issues in $(basename "$file"):"
            if [[ -s "$temp_file" ]]; then
                yellow "$(cat "$temp_file")"
            else
                yellow "  File needs formatting"
            fi
        fi
        
        rm -f "$temp_file"
    done
    
    if [[ "$has_errors" == "true" ]]; then
        echo
        red "Code formatting issues found in ${#error_files[@]} files!"
        
        if [[ "$VERBOSE" == "true" ]]; then
            echo "Files with issues:"
            printf '  %s\n' "${error_files[@]}"
        fi
        
        echo
        echo "To fix these issues, run:"
        echo "  $0"
        exit 1
    else
        green "All files are properly formatted!"
        exit 0
    fi
else
    blue "Applying code formatting..."
    
    formatted_count=0
    
    for file in "${files[@]}"; do
        log_verbose "Formatting: $file"
        
        # Create a backup of the original file to check if it changed
        temp_backup=$(mktemp)
        cp "$file" "$temp_backup"
        
        # Apply formatting
        if clang-format --style=file -i "$file"; then
            # Check if the file actually changed
            if ! cmp -s "$file" "$temp_backup"; then
                echo "Formatted: $(basename "$file")"
                ((formatted_count++))
            else
                log_verbose "No changes: $(basename "$file")"
            fi
        else
            red "Error formatting: $file"
        fi
        
        rm -f "$temp_backup"
    done
    
    echo
    if [[ $formatted_count -gt 0 ]]; then
        green "Code formatting complete! $formatted_count files were modified."
    else
        green "Code formatting complete! All files were already properly formatted."
    fi
fi