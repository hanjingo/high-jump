# Benchmark runner script with timestamped results export and optional comparison for Windows PowerShell
# Usage: .\run_benchmark.ps1 [-Compare] [benchmark_options]

param(
    [switch]$Compare,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$BenchmarkArgs
)

# Get the directory of this script
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ResultsDir = Join-Path $ScriptDir "results"
$BinDir = Join-Path (Split-Path -Parent $ScriptDir) "bin"

# Create results directory if it doesn't exist
if (!(Test-Path $ResultsDir)) {
    New-Item -ItemType Directory -Path $ResultsDir | Out-Null
}

# Generate timestamp
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$JsonFile = Join-Path $ResultsDir "benchmark_$Timestamp.json"
$TxtFile = Join-Path $ResultsDir "benchmark_$Timestamp.txt"

# Check if benchmark binary exists
$BenchmarkBin = Join-Path $BinDir "benchs.exe"
if (!(Test-Path $BenchmarkBin)) {
    Write-Host "Error: Benchmark binary not found at $BenchmarkBin" -ForegroundColor Red
    Write-Host "Please build the project first with: cmake --build . --target benchs" -ForegroundColor Yellow
    exit 1
}

Write-Host "Running benchmarks..." -ForegroundColor Green
Write-Host "Timestamp: $Timestamp"
Write-Host "Results will be saved to:"
Write-Host "  JSON: $JsonFile" -ForegroundColor Cyan
Write-Host "  Text: $TxtFile" -ForegroundColor Cyan
Write-Host ""

# Prepare arguments
$Args = @("--benchmark_min_time=0.1", "--benchmark_format=json", "--benchmark_out=$JsonFile")
if ($BenchmarkArgs) {
    $Args += $BenchmarkArgs
}

try {
    # Run benchmark with both console output and JSON export
    # Console output is also saved to text file for reference
    & $BenchmarkBin $Args | Tee-Object -FilePath $TxtFile
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "✓ Benchmark completed successfully!" -ForegroundColor Green
        Write-Host "Results saved to:"
        Write-Host "  JSON: $JsonFile" -ForegroundColor Cyan
        Write-Host "  Text: $TxtFile" -ForegroundColor Cyan
        
        # Show file sizes for reference
        Write-Host ""
        Write-Host "File sizes:"
        $JsonSize = (Get-Item $JsonFile).Length
        $TxtSize = (Get-Item $TxtFile).Length
        Write-Host "  $(Split-Path -Leaf $JsonFile): $JsonSize bytes"
        Write-Host "  $(Split-Path -Leaf $TxtFile): $TxtSize bytes"
        
        # Run comparison if requested
        if ($Compare) {
            Write-Host ""
            Write-Host "Running comparison with previous results..." -ForegroundColor Green
            
            # Check if generic tool exists
            $GenericTool = Join-Path (Split-Path -Parent $ScriptDir) "scripts\benchmark.py"
            if (Test-Path $GenericTool) {
                $CompareArgs = @(
                    "compare"
                    "--results-dir=results"
                    "--file-pattern=benchmark_(\d{8}_\d{6})\.json"
                    "--timestamp-format=%Y%m%d_%H%M%S"
                    "--name-prefix=BM_"
                    "--threshold=1.0"
                    "--run-command=.\run_benchmark.ps1"
                )
                & python $GenericTool $CompareArgs
            } else {
                Write-Host "Warning: Generic benchmark tool not found at $GenericTool" -ForegroundColor Yellow
            }
        } else {
            Write-Host ""
            Write-Host "To compare with previous results, run:"
            Write-Host "  .\run_benchmark.ps1 -Compare" -ForegroundColor Yellow
            Write-Host ""
            Write-Host "Or manually compare with:"
            Write-Host "  python ..\scripts\benchmark.py compare \" -ForegroundColor Yellow
            Write-Host "    --results-dir=results \" -ForegroundColor Yellow
            Write-Host "    --file-pattern='benchmark_(\d{8}_\d{6})\.json' \" -ForegroundColor Yellow
            Write-Host "    --timestamp-format='%Y%m%d_%H%M%S' \" -ForegroundColor Yellow
            Write-Host "    --name-prefix='BM_' \" -ForegroundColor Yellow
            Write-Host "    --threshold=1.0 \" -ForegroundColor Yellow
            Write-Host "    --run-command='.\run_benchmark.ps1'" -ForegroundColor Yellow
        }
    } else {
        Write-Host ""
        Write-Host "✗ Benchmark failed!" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host ""
    Write-Host "✗ Error running benchmark: $_" -ForegroundColor Red
    exit 1
}