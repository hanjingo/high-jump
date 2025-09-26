# Benchmark Utility Tool (benchmark.py)

A multi-purpose benchmark utility that provides various subcommands for benchmark result analysis.

## Overview

The `benchmark.py` script has been redesigned as a comprehensive benchmark utility with multiple subcommands:

- **compare**: Compare two benchmark result files (default behavior for backward compatibility)
- **analyze**: Analyze single benchmark result file with detailed statistics
- **list**: List all available benchmark result files in the results directory

## Usage

### Compare Subcommand (Default)

Compare benchmark results between two files:

```bash
# Compare most recent results automatically
python3 benchmark.py compare

# Compare specific files
python3 benchmark.py compare new.json old.json

# Compare with custom settings
python3 benchmark.py compare --threshold=2.0 --higher-is-better
```

### Analyze Subcommand

Analyze a single benchmark result file:

```bash
# Analyze most recent result file
python3 benchmark.py analyze

# Analyze specific file
python3 benchmark.py analyze results/benchmark_20250927_143022.json

# Analyze with custom display options
python3 benchmark.py analyze --no-color --name-prefix="Test_"
```

### List Subcommand

List all available benchmark result files:

```bash
# List files in default results directory
python3 benchmark.py list

# List files in custom directory
python3 benchmark.py list --results-dir=output

# List files with custom pattern
python3 benchmark.py list --file-pattern="test_(\d+)\.json"
```

## Backward Compatibility

The tool maintains backward compatibility with the old `compare_benchmark_results.py` script:

```bash
# Old way (still works)
python3 benchmark.py --results-dir=results --threshold=1.5

# New way (recommended)
python3 benchmark.py compare --results-dir=results --threshold=1.5
```

## Integration

This tool is automatically called by the `run_benchmark` scripts when using the `--compare` option:

```bash
# Shell script calls: python3 ../scripts/benchmark.py compare [options]
./run_benchmark.sh --compare

# Batch script calls: python ..\scripts\benchmark.py compare [options]  
run_benchmark.bat /compare

# PowerShell script calls: python ..\scripts\benchmark.py compare [options]
.\run_benchmark.ps1 -Compare
```

## Available Options

### Common Options (All Subcommands)

- `--results-dir`: Directory to search for result files
- `--file-pattern`: Regex pattern to match result files
- `--timestamp-format`: Format for parsing timestamps from filenames
- `--no-color`: Disable colored output
- `--name-prefix`: Prefix to remove from benchmark names
- `--run-command`: Command suggestion for running benchmarks

### Compare-Specific Options

- `--threshold`: Minimum percentage change to be considered significant
- `--higher-is-better`: Higher values indicate better performance

## Examples

### Development Workflow

```bash
# Run benchmarks
./run_benchmark.sh

# List available results
python3 ../scripts/benchmark.py list

# Analyze latest result
python3 ../scripts/benchmark.py analyze

# Make code changes, then run with comparison
./run_benchmark.sh --compare
```

### Manual Analysis

```bash
# Compare specific versions
python3 benchmark.py compare \
    results/benchmark_20250927_143022.json \
    results/benchmark_20250927_142015.json \
    --threshold=0.5

# Analyze with custom settings
python3 benchmark.py analyze \
    results/latest.json \
    --name-prefix="BM_" \
    --no-color
```

### CI/CD Integration

```bash
# List results for logging
python3 benchmark.py list --no-color > benchmark_files.log

# Run comparison with specific settings
python3 benchmark.py compare \
    --threshold=5.0 \
    --higher-is-better \
    --no-color \
    > comparison_report.txt
```