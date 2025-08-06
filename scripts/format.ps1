#!/usr/bin/env pwsh
# filepath: c:\work\libcpp\scripts\format.ps1

<#
.SYNOPSIS
    Format C++ code using clang-format

.PARAMETER Check
    Only check formatting without making changes
#>

param(
    [switch]$Check
)

# Ensure we're in the project root
if (!(Test-Path ".clang-format")) {
    Write-Error "Must be run from project root (directory containing .clang-format)"
    exit 1
}

# Check if clang-format is available
try {
    clang-format --version | Out-Null
} catch {
    Write-Error "clang-format not found. Please install clang-format."
    exit 1
}

# Find all C++ files
$files = Get-ChildItem -Recurse -Path "libcpp", "tests" -Include "*.cpp", "*.hpp", "*.h" -ErrorAction SilentlyContinue

if ($files.Count -eq 0) {
    Write-Warning "No C++ files found"
    exit 0
}

Write-Host "Found $($files.Count) C++ files"

if ($Check) {
    Write-Host "Checking code formatting..."
    $hasErrors = $false
    
    foreach ($file in $files) {
        $result = clang-format --style=file --dry-run --Werror $file.FullName 2>&1
        if ($result) {
            Write-Host "Format issues in $($file.Name):" -ForegroundColor Red
            Write-Host $result -ForegroundColor Yellow
            $hasErrors = $true
        }
    }
    
    if ($hasErrors) {
        Write-Host "Code formatting issues found!" -ForegroundColor Red
        exit 1
    } else {
        Write-Host "All files are properly formatted!" -ForegroundColor Green
        exit 0
    }
} else {
    Write-Host "Applying code formatting..."
    
    foreach ($file in $files) {
        Write-Host "Formatting: $($file.Name)"
        clang-format --style=file -i $file.FullName
    }
    
    Write-Host "Code formatting complete!" -ForegroundColor Green
}