# Code Formatting Scripts

This directory contains cross-platform code formatting scripts for the high-jump C++ project using clang-format.

## Overview

The formatting scripts provide a unified interface for code formatting across different platforms:
- `format.sh` - Cross-platform Bash script for Linux and macOS
- `format.ps1` - PowerShell script for Windows

## Prerequisites

### All Platforms
- **clang-format** must be installed and available in PATH
- Must be executed from the project root directory (containing `.clang-format`)

### Installation Instructions

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install clang-format

# CentOS/RHEL
sudo yum install clang-tools-extra

# Fedora
sudo dnf install clang-tools-extra

# Arch Linux
sudo pacman -S clang
```

#### macOS
```bash
# Using Homebrew
brew install clang-format

# Or install Xcode Command Line Tools
xcode-select --install
```

#### Windows
```powershell
# Using Chocolatey
choco install llvm

# Using Scoop
scoop install llvm

# Or download from LLVM releases
# https://github.com/llvm/llvm-project/releases
```

## Usage

### Linux/macOS (format.sh)

```bash
# Format all C++ files
./scripts/format.sh

# Check formatting without making changes
./scripts/format.sh --check
./scripts/format.sh -c

# Enable verbose output
./scripts/format.sh --verbose
./scripts/format.sh -v

# Combine options
./scripts/format.sh --check --verbose

# Format specific files
./scripts/format.sh file1.hpp file2.cpp

# Check specific file with detailed output
./scripts/format.sh --check --verbose hj/net/tcp/tcp_socket.hpp
```

### Windows (format.ps1)

```powershell
# Format all C++ files
./scripts/format.ps1

# Check formatting without making changes
./scripts/format.ps1 -Check
```

## Options

### format.sh Options

| Option | Short | Description |
|--------|-------|-------------|
| `--check` | `-c` | Only check formatting without making changes |
| `--verbose` | `-v` | Enable verbose output showing detailed information |
| `--help` | `-h` | Show help message and usage information |

### format.sh Arguments

| Argument | Description |
|----------|-------------|
| `FILES...` | Specific files to format (optional, defaults to all C++ files) |

### format.ps1 Options

| Parameter | Description |
|-----------|-------------|
| `-Check` | Only check formatting without making changes |

## File Coverage

Both scripts process C++ source files in the following directories:
- `hj/` - Header files and implementation
- `tests/` - Test files

### Supported File Extensions
- `.cpp` - C++ source files
- `.hpp` - C++ header files
- `.h` - C header files
- `.cc` - C++ source files (alternative extension)
- `.cxx` - C++ source files (alternative extension)

## Behavior

### Format Mode (Default)
- Scans for C++ files in target directories
- Applies clang-format to each file in-place
- Reports which files were modified
- Shows total count of formatted files

### Check Mode
- Validates formatting without making changes
- Reports files that need formatting
- Exits with error code if formatting issues found
- Perfect for CI/CD pipelines and pre-commit hooks

## Output Examples

### Successful Formatting
```bash
$ ./scripts/format.sh
Found 42 C++ files
Applying code formatting...
Formatted: astar.hpp
Formatted: matrix.hpp
Code formatting complete! 2 files were modified.
```

### Check Mode - Issues Found
```bash
$ ./scripts/format.sh --check
Found 42 C++ files
Checking code formatting...
Format issues in astar.hpp:
  File needs formatting
Code formatting issues found in 1 files!

To fix these issues, run:
  ./scripts/format.sh
```

### Check Mode - All Good
```bash
$ ./scripts/format.sh --check
Found 42 C++ files
Checking code formatting...
All files are properly formatted!
```

## Integration

### Pre-commit Hook
Add to `.git/hooks/pre-commit`:
```bash
#!/bin/bash
cd "$(git rev-parse --show-toplevel)"
./scripts/format.sh --check
```

### GitHub Actions
```yaml
- name: Check Code Formatting
  run: ./scripts/format.sh --check
```

### VS Code Task
Add to `.vscode/tasks.json`:
```json
{
    "label": "Format C++ Code",
    "type": "shell",
    "command": "./scripts/format.sh",
    "group": "build",
    "presentation": {
        "echo": true,
        "reveal": "always"
    }
}
```

## Configuration

The formatting behavior is controlled by the `.clang-format` file in the project root. This ensures consistent formatting rules across all team members and environments.

## Error Handling

### Common Issues

1. **clang-format not found**
   - Install clang-format following platform-specific instructions
   - Ensure it's available in PATH

2. **Not in project root**
   - Navigate to directory containing `.clang-format`
   - Scripts validate presence of configuration file

3. **Permission denied**
   - Ensure scripts have execute permissions:
     ```bash
     chmod +x scripts/format.sh
     ```

4. **No files found**
   - Verify `hj/` and `tests/` directories exist
   - Check if directories contain C++ files

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success - all files properly formatted |
| 1 | Error - formatting issues found or tool failure |

## Best Practices

1. **Run before committing**: Always format code before creating commits
2. **Use check mode in CI**: Validate formatting in continuous integration
3. **Team consistency**: Ensure all team members use the same clang-format version
4. **IDE integration**: Configure your IDE to use the same formatting rules

## Troubleshooting

### Verbose Mode
Use verbose mode to debug issues:
```bash
./scripts/format.sh --check --verbose
```

This provides detailed information about:
- Project root detection
- clang-format version
- Files being processed
- Detailed error messages

### Manual Formatting
For individual files:
```bash
clang-format --style=file -i path/to/file.cpp
```

### Version Compatibility
Different clang-format versions may produce slightly different results. For team consistency, document the recommended version in your project documentation.