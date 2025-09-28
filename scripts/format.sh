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
Usage: $0 [OPTIONS] [FILES...]

Format C++ code using clang-format

Options:
    -c, --check     Only check formatting without making changes
    -h, --help      Show this help message
    -v, --verbose   Enable verbose output

Arguments:
    FILES...        Specific files to format (optional, defaults to all C++ files)

Examples:
    $0                              Format all C++ files
    $0 --check                      Check formatting of all files
    $0 -c -v                        Check formatting with verbose output
    $0 file1.hpp file2.cpp          Format specific files
    $0 --check --verbose file.hpp   Check specific file with details
EOF
}

# Parse command line arguments
CHECK_ONLY=false
VERBOSE=false
SPECIFIED_FILES=()

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
        -*)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
        *)
            # This is a file argument
            SPECIFIED_FILES+=("$1")
            shift
            ;;
    esac
done

# Function to log verbose messages
log_verbose() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo "$@"
    fi
}

# Function to detect auto-generated files
is_autogen_file() {
    case "$1" in
        *.pb.h|*.pb.cc|*.pb.c|*.grpc.pb.h|*.grpc.pb.cc|*.grpc.pb.c|*generated.h)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
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

# Determine which files to process
if [[ ${#SPECIFIED_FILES[@]} -gt 0 ]]; then
    log_verbose "Processing specified files..."
    files=()
    for file in "${SPECIFIED_FILES[@]}"; do
        if [[ -f "$file" ]]; then
            case "$file" in
                *.cpp|*.hpp|*.h|*.cc|*.cxx)
                    files+=("$file")
                    ;;
                *)
                    yellow "Warning: Skipping non-C++ file: $file"
                    ;;
            esac
        else
            red "Error: File not found: $file"
            exit 1
        fi
    done
else
    log_verbose "Searching for C++ files in hj/ and tests/ directories..."
    files=()
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find hj tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" -o -name "*.cxx" \) -print0 2>/dev/null || true)
fi

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
        if is_autogen_file "$file"; then
            log_verbose "Skipping auto-generated file: $file"
            continue
        fi

        log_verbose "Checking: $file"
        temp_file=$(mktemp)
        if ! clang-format --style=file --dry-run --Werror "$file" 2>"$temp_file"; then
            has_errors=true
            error_files+=("$file")

            red "Format issues in $(basename "$file"):"
            if [[ -s "$temp_file" ]]; then
                echo "  Specific format issues:"
                cat "$temp_file" | head -10 | sed "s|$file:|Line |g" | while read -r line; do
                    case "$line" in
                        *"error: code should be clang-formatted"*) 
                            echo "    $(echo "$line" | sed 's/error: code should be clang-formatted.*/does not conform to formatting standards/')"
                            ;;
                        *) 
                            echo "    $line"
                            ;;
                    esac
                done | head -5
                if [[ $(cat "$temp_file" | wc -l) -gt 5 ]]; then
                    yellow "  ... (showing first 5 issue locations, total $(cat "$temp_file" | wc -l) format issues)"
                fi
            else
                yellow "  File needs formatting (unable to show details)"
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
    modified_files=()
    for file in "${files[@]}"; do
        if is_autogen_file "$file"; then
            log_verbose "Skipping auto-generated file: $file"
            continue
        fi

        log_verbose "Formatting: $file"
        temp_backup=$(mktemp)
        cp "$file" "$temp_backup"

        clang-format --style=file -i "$file"
        if [[ $? -eq 0 ]]; then
            if ! cmp -s "$file" "$temp_backup"; then
                echo "Formatted: $file"
                modified_files+=("$file")
            else
                log_verbose "No changes: $file"
            fi
        else
            red "Error formatting: $file"
        fi

        rm -f "$temp_backup"
    done

    echo
    if [[ ${#modified_files[@]} -gt 0 ]]; then
        green "Code formatting complete! ${#modified_files[@]} files were modified."
    else
        green "Code formatting complete! All files were already properly formatted."
    fi
fi
