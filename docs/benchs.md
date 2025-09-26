# Benchmark Tools

Integrated benchmark runner with automatic comparison capabilities.

## Local Testing

The `results/` directory is automatically created for storing benchmark results during local testing. This directory is ignored by git since benchmark results vary significantly between different machines, CPU architectures, and system configurations. Results are intended for local performance analysis and comparison only.

## Scripts

- `run_benchmark.sh` - Shell script for Unix/Linux/macOS systems with integrated comparison
- `run_benchmark.bat` - Batch script for Windows systems with integrated comparison
- `run_benchmark.ps1` - PowerShell script for Windows systems with integrated comparison (recommended)
- `../scripts/benchmark.py` - Generic benchmark utility tool (used internally)
- `../scripts/setup_powershell_policy.bat` - Generic PowerShell policy setup tool (batch)
- `../scripts/setup_powershell_policy.ps1` - Generic PowerShell policy setup tool (PowerShell)

## Usage

### Running Benchmarks

1. **Build the project first:**
   ```bash
   # From the root directory (Unix/Linux/macOS)
   cmake -B build
   cmake --build build --target benchs
   ```
   
   ```cmd
   :: From the root directory (Windows Command Prompt)
   cmake -B build
   cmake --build build --target benchs
   ```
   
   ```powershell
   # From the root directory (Windows PowerShell)
   cmake -B build
   cmake --build build --target benchs
   ```

2. **Run benchmarks:**

   **Basic benchmark run (saves results only):**
   
   **Unix/Linux/macOS:**
   ```bash
   # Run from benchs directory
   ./run_benchmark.sh [benchmark_options]
   ```
   
   **Windows Command Prompt:**
   ```cmd
   :: Run from benchs directory  
   run_benchmark.bat [benchmark_options]
   ```
   
   **Windows PowerShell:**
   ```powershell
   # Run from benchs directory
   .\run_benchmark.ps1 [benchmark_options]
   ```

   **Benchmark run with automatic comparison:**
   
   **Unix/Linux/macOS:**
   ```bash
   # Run from benchs directory
   ./run_benchmark.sh --compare [benchmark_options]
   ```
   
   **Windows Command Prompt:**
   ```cmd
   :: Run from benchs directory  
   run_benchmark.bat /compare [benchmark_options]
   ```
   
   **Windows PowerShell:**
   ```powershell
   # Run from benchs directory
   .\run_benchmark.ps1 -Compare [benchmark_options]
   ```

### Examples

**Basic benchmark run:**
```bash
./run_benchmark.sh
```

**Run with comparison:**
```bash
./run_benchmark.sh --compare
```

**Run specific benchmarks with comparison:**
```bash
./run_benchmark.sh --compare --benchmark_filter="BM_Defer.*"
```

**Windows equivalents:**
```cmd
run_benchmark.bat /compare --benchmark_filter="BM_Defer.*"
```
```powershell
.\run_benchmark.ps1 -Compare --benchmark_filter="BM_Defer.*"
```

### Features

- **Timestamped Results**: Automatically generates timestamp-based filenames (`benchmark_YYYYMMDD_HHMMSS.json`)
- **Dual Output**: Saves both JSON (for comparison) and text (for human reading) formats  
- **Cross-Platform**: Consistent functionality across Linux, macOS, and Windows
- **Integrated Comparison**: Optional flag to run comparison immediately after benchmarking
- **Auto-Detection**: Automatically finds and compares with the most recent previous result
- **Error Handling**: Clear error messages with helpful suggestions
- **File Size Reporting**: Shows result file sizes for reference

### Workflow

1. **First Run**: `./run_benchmark.sh` - Creates initial baseline
2. **Make Changes**: Modify code, settings, or environment  
3. **Compare Run**: `./run_benchmark.sh --compare` - Runs benchmarks and shows comparison
4. **Iterate**: Repeat steps 2-3 for continuous performance monitoring

## Manual Comparison

If you prefer manual control over comparisons, you can use the generic benchmark tool directly:

```bash
python3 ../scripts/benchmark.py compare \
    --results-dir=results \
    --file-pattern='benchmark_(\d{8}_\d{6})\.json' \
    --timestamp-format='%Y%m%d_%H%M%S' \
    --name-prefix='BM_' \
    --threshold=1.0 \
    --run-command='./run_benchmark.sh'
```

You can also use other subcommands:

```bash
# Analyze single result file
python3 ../scripts/benchmark.py analyze results/benchmark_20250927_143022.json

# List all available result files
python3 ../scripts/benchmark.py list

# Get help for any subcommand
python3 ../scripts/benchmark.py compare --help
```

## Output Examples

### Basic Run Output

```
Running benchmarks...
Timestamp: 20250927_143022
Results will be saved to:
  JSON: results/benchmark_20250927_143022.json
  Text: results/benchmark_20250927_143022.txt

[Benchmark execution...]

✓ Benchmark completed successfully!
Results saved to:
  JSON: results/benchmark_20250927_143022.json
  Text: results/benchmark_20250927_143022.txt

File sizes:
  benchmark_20250927_143022.json: 15234 bytes
  benchmark_20250927_143022.txt: 8765 bytes

To compare with previous results, run:
  ./run_benchmark.sh --compare
```

### Comparison Run Output

```
Running benchmarks...
Timestamp: 20250927_143022
[benchmark execution...]

✓ Benchmark completed successfully!

Running comparison with previous results...
High-Jump Benchmark Results Comparison
=====================================

Comparing: benchmark_20250927_143022.json vs benchmark_20250927_142015.json

Performance Changes:
  BM_DeferFunction/1024        -2.45%  (2.142ms → 2.090ms)  IMPROVEMENT
  BM_DeferLambda/1024          +1.23%  (1.987ms → 2.012ms)  REGRESSION
  BM_StandardDestructor/1024   -0.87%  (1.456ms → 1.444ms)  STABLE

Summary: 1 improvement, 1 regression, 1 stable
```

## CMake Targets

The CMakeLists.txt also provides these targets:

- `cmake --build build --target benchmark_console` - Run with console output only
- `cmake --build build --target benchmark_run` - Run with JSON export (requires TIMESTAMP environment variable)

## Requirements

- C++17 compiler
- Google Benchmark library  
- Python 3.6+ (for comparison script)
- **Unix/Linux/macOS:** Bash shell
- **Windows:** Command Prompt or PowerShell (PowerShell recommended for better Unicode support)

## File Naming Convention

Result files are automatically named with timestamps:
- Format: `benchmark_YYYYMMDD_HHMMSS.json`
- Example: `benchmark_20250927_143022.json`

This ensures chronological ordering and prevents file conflicts.

## Windows-Specific Notes

### PowerShell Execution Policy
If you encounter execution policy errors when running `run_benchmark.ps1`, you can either:

1. **Use the generic PowerShell setup tool:**
   ```cmd
   ..\scripts\setup_powershell_policy.bat
   ```
   ```powershell
   ..\scripts\setup_powershell_policy.ps1
   ```

2. **Manually set execution policy:**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

3. **Use batch script instead:**
   ```cmd
   run_benchmark.bat
   ```

### Path Separators
- The Python comparison script automatically handles both Unix (`/`) and Windows (`\`) path separators
- You can use either format when specifying file paths

### Unicode Support
- PowerShell script provides better Unicode support for benchmark names and output
- Batch script may have issues with special characters in some console configurations