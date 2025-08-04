param(
    [switch]$Check = $false,
    [switch]$Verbose = $false,
    [switch]$Staged = $false
)

Write-Host "Code Formatting Tool" -ForegroundColor Green

# check clang-format
if (-not (Get-Command "clang-format" -ErrorAction SilentlyContinue)) {
    Write-Host "clang-format not found!" -ForegroundColor Red
    Write-Host "Install options:" -ForegroundColor Yellow
    Write-Host "  - Visual Studio with C++ tools" -ForegroundColor White
    Write-Host "  - LLVM: https://llvm.org/builds/" -ForegroundColor White
    exit 1
}

$directories = @("libcpp", "test")
$extensions = @("*.cpp", "*.hpp", "*.h")

$files = @()

if ($Staged) {
    # Check staged files only
    Write-Host "Checking staged files only..." -ForegroundColor Cyan
    $stagedFiles = git diff --cached --name-only --diff-filter=ACMR | Where-Object { $_ -match '\.(cpp|hpp|h)$' }
    
    if ($stagedFiles) {
        foreach ($file in $stagedFiles) {
            if (Test-Path $file) {
                $files += Get-Item $file
            }
        }
    }
} else {
    # Check all files in specified directories
    Write-Host "Checking all files in directories: $($directories -join ', ')" -ForegroundColor Cyan
    foreach ($dir in $directories) {
        if (Test-Path $dir) {
            $dirFiles = Get-ChildItem -Path $dir -Include $extensions -Recurse -File
            $files += $dirFiles
        }
    }
}

if ($files.Count -eq 0) {
    if ($Staged) {
        Write-Host "No staged C++ files found" -ForegroundColor Yellow
    } else {
        Write-Host "No C++ files found" -ForegroundColor Yellow
    }
    exit 0
}

Write-Host "Found $($files.Count) files to process" -ForegroundColor Cyan

if ($Check) {
    Write-Host "`nChecking formatting..." -ForegroundColor Yellow
    $needsFormatting = @()
    
    foreach ($file in $files) {
        $result = clang-format --dry-run --Werror $file.FullName 2>&1
        if ($LASTEXITCODE -ne 0) {
            $needsFormatting += $file
            if ($Verbose) {
                Write-Host "$($file.Name)" -ForegroundColor Red
            }
        } elseif ($Verbose) {
            Write-Host "$($file.Name)" -ForegroundColor Green
        }
    }
    
    if ($needsFormatting.Count -gt 0) {
        Write-Host "`n$($needsFormatting.Count) files need formatting:" -ForegroundColor Red
        foreach ($file in $needsFormatting) {
            # Use Resolve-Path to get relative path
            $relativePath = Resolve-Path -Path $file.FullName -Relative
            Write-Host "  - $relativePath" -ForegroundColor Yellow
        }
        Write-Host "`nTo fix formatting:" -ForegroundColor White
        Write-Host "  .\script\format.ps1" -ForegroundColor Cyan
        if ($Staged) {
            Write-Host "  .\script\format.ps1 -Staged  # Format staged files only" -ForegroundColor Cyan
        }
        exit 1
    } else {
        Write-Host "`nAll files are properly formatted!" -ForegroundColor Green
        exit 0
    }
} else {
    Write-Host "`nFormatting files..." -ForegroundColor Yellow
    $formatted = 0
    
    foreach ($file in $files) {
        if ($Verbose) {
            $relativePath = Resolve-Path -Path $file.FullName -Relative
            Write-Host "  Processing: $relativePath" -ForegroundColor Gray
        }
        
        clang-format -i $file.FullName
        if ($LASTEXITCODE -eq 0) {
            $formatted++
        }
    }
    
    Write-Host "`nFormatted $formatted files successfully!" -ForegroundColor Green
    
    if ($Staged) {
        Write-Host "Don't forget to re-stage the formatted files:" -ForegroundColor Yellow
        Write-Host "   git add <files>" -ForegroundColor Cyan
    }
}