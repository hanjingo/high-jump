# coverage.ps1 - Project-independent coverage runner for Windows
# Usage:
#   .\coverage.ps1 -Edition <Community|Enterprise|Professional> -Exe <test_exe_path> -Source <source_root> [-CoverageExe <OpenCppCoverage.exe>] [-WorkDir <work_dir>] [-VsInstr <vsinstr.exe>] [-VsPerfCmd <vsperfcmd.exe>] [-VsPerfCov <vsperfcov.exe>]

param(
    [string]$Edition = "Community",
    [string]$Exe,
    [string]$Source,
    [string]$CoverageExe = "OpenCppCoverage.exe",
    [string]$WorkDir,
    [string]$VsInstr = "vsinstr.exe",
    [string]$VsPerfCmd = "vsperfcmd.exe",
    [string]$VsPerfCov = "vsperfcov.exe"
)

if (-not $Exe) {
    Write-Host "Error: -Exe <test_exe_path> is required."
    exit 1
}
if (-not $Source) {
    Write-Host "Error: -Source <source_root> is required."
    exit 1
}
if (-not $WorkDir) {
    $WorkDir = Split-Path -Parent $Exe
}

Push-Location $WorkDir

switch ($Edition) {
    "Community" {
        Write-Host "Using OpenCppCoverage for coverage..."
        $coveragePath = (Get-Command $CoverageExe -ErrorAction SilentlyContinue).Source
        if (-not $coveragePath) {
            Write-Host "$CoverageExe not found in PATH. Please install and add to PATH."
            Pop-Location
            exit 1
        }
        & $CoverageExe --sources "$Source" --export_type html --export_type cobertura -- $(Split-Path -Leaf $Exe)
    }
    "Enterprise" {
        Write-Host "Using Visual Studio Enterprise coverage tools..."
        & $VsInstr /coverage $(Split-Path -Leaf $Exe)
        & $VsPerfCmd /start:coverage /output:tests.coverage
        & $(Split-Path -Leaf $Exe)
        & $VsPerfCmd /shutdown
        & $VsPerfCov tests.coverage /output:tests.coverage.xml
        Write-Host "Coverage report generated: tests.coverage.xml"
    }
    "Professional" {
        Write-Host "Using Visual Studio Professional coverage tools..."
        & $VsInstr /coverage $(Split-Path -Leaf $Exe)
        & $VsPerfCmd /start:coverage /output:tests.coverage
        & $(Split-Path -Leaf $Exe)
        & $VsPerfCmd /shutdown
        & $VsPerfCov tests.coverage /output:tests.coverage.xml
        Write-Host "Coverage report generated: tests.coverage.xml"
    }
    default {
        Write-Host "Unknown edition, defaulting to OpenCppCoverage..."
        $coveragePath = (Get-Command $CoverageExe -ErrorAction SilentlyContinue).Source
        if (-not $coveragePath) {
            Write-Host "$CoverageExe not found in PATH. Please install and add to PATH."
            Pop-Location
            exit 1
        }
        & $CoverageExe --sources "$Source" --export_type html --export_type cobertura -- $(Split-Path -Leaf $Exe)
    }
}

Pop-Location
