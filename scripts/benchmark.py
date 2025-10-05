#!/usr/bin/env python3
"""
Generic Benchmark Utility Tool

This is a multi-purpose benchmark utility that provides various subcommands:
- compare: Compare two benchmark result files (JSON format)
- analyze: Analyze single benchmark result file
- list: List available benchmark result files

Usage:
    python3 benchmark.py compare [options] [new_result.json] [old_result.json]
    python3 benchmark.py analyze [options] [result.json]
    python3 benchmark.py list [options]
    
The tool supports Google Benchmark JSON format and can be configured for different 
file naming patterns and comparison criteria.
"""

import json
import sys
import os
import argparse
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import re

# Configuration class for customization
class ComparisonConfig:
    def __init__(self):
        # File patterns and directories
        self.results_dir = "results"
        self.file_pattern = r'benchmark_(\d{8}_\d{6})\.json'  # Default: benchmark_YYYYMMDD_HHMMSS.json
        self.timestamp_format = '%Y%m%d_%H%M%S'
        
        # Benchmark name formatting
        self.name_prefix_to_remove = "bm_"  # Remove "bm_" prefix from benchmark names
        self.name_separator_replacement = "_"  # Replace "_" with " " in names
        
        # Performance analysis settings
        self.significant_change_threshold = 1.0  # Minimum % change to be considered significant
        self.improvement_means_lower = True  # True for timing benchmarks (lower is better)
        
        # Display settings
        self.max_summary_items = 3  # Show top N improvements/regressions in summary
        
        # Run command suggestion
        self.run_command_template = "./run_benchmark.sh"  # Command to suggest for running benchmarks

    def from_args(self, args):
        """Update configuration from command line arguments."""
        if hasattr(args, 'results_dir') and args.results_dir:
            self.results_dir = args.results_dir
        if hasattr(args, 'file_pattern') and args.file_pattern:
            self.file_pattern = args.file_pattern
        if hasattr(args, 'timestamp_format') and args.timestamp_format:
            self.timestamp_format = args.timestamp_format
        if hasattr(args, 'name_prefix') and args.name_prefix:
            self.name_prefix_to_remove = args.name_prefix
        if hasattr(args, 'threshold') and args.threshold:
            self.significant_change_threshold = args.threshold
        if hasattr(args, 'higher_is_better') and args.higher_is_better:
            self.improvement_means_lower = False
        if hasattr(args, 'run_command') and args.run_command:
            self.run_command_template = args.run_command

# Color codes for terminal output
class Colors:
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

def load_benchmark_results(filepath: str) -> Optional[Dict]:
    """Load benchmark results from JSON file."""
    try:
        with open(filepath, 'r') as f:
            data = json.load(f)
        return data
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"{Colors.RED}Error loading {filepath}: {e}{Colors.END}")
        return None

def extract_timestamp_from_filename(filename: str, config: ComparisonConfig) -> Optional[datetime]:
    """Extract timestamp from benchmark filename using configurable pattern."""
    match = re.search(config.file_pattern, filename)
    if match:
        try:
            return datetime.strptime(match.group(1), config.timestamp_format)
        except ValueError:
            pass
    return None

def find_most_recent_result(config: ComparisonConfig) -> Optional[str]:
    """Find the most recent benchmark result file in the configured directory."""
    results_path = Path(config.results_dir)
    if not results_path.exists():
        return None
    
    candidates = []
    for file_path in results_path.glob('*.json'):
        timestamp = extract_timestamp_from_filename(file_path.name, config)
        if timestamp:
            candidates.append((timestamp, str(file_path)))
    
    if candidates:
        # Return the most recent one
        candidates.sort(key=lambda x: x[0], reverse=True)
        return candidates[0][1]
    
    return None

def find_most_recent_previous_result(current_file: str, config: ComparisonConfig) -> Optional[str]:
    """Find the most recent benchmark result file before the current one."""
    current_path = Path(current_file)
    results_dir = current_path.parent
    current_timestamp = extract_timestamp_from_filename(current_path.name, config)
    
    if not current_timestamp:
        return None
    
    candidates = []
    for file_path in results_dir.glob('*.json'):
        if file_path.name == current_path.name:
            continue
        
        timestamp = extract_timestamp_from_filename(file_path.name, config)
        if timestamp and timestamp < current_timestamp:
            candidates.append((timestamp, str(file_path)))
    
    if candidates:
        # Return the most recent one
        candidates.sort(key=lambda x: x[0], reverse=True)
        return candidates[0][1]
    
    return None

def format_time_unit(time_unit: str) -> str:
    """Format time unit for display."""
    unit_map = {
        'ns': 'nanoseconds',
        'us': 'microseconds', 
        'ms': 'milliseconds',
        's': 'seconds'
    }
    return unit_map.get(time_unit, time_unit)

def format_number(num: float) -> str:
    """Format number with appropriate precision."""
    if num >= 1000:
        return f"{num:,.0f}"
    elif num >= 100:
        return f"{num:.1f}"
    elif num >= 10:
        return f"{num:.2f}"
    else:
        return f"{num:.3f}"

def calculate_change_percentage(old_val: float, new_val: float) -> float:
    """Calculate percentage change between two values."""
    if old_val == 0:
        return float('inf') if new_val > 0 else 0
    return ((new_val - old_val) / old_val) * 100

def format_percentage_change(change: float, config: ComparisonConfig) -> str:
    """Format percentage change with appropriate color coding."""
    if abs(change) < 0.1:
        return f"{Colors.WHITE}no change{Colors.END}"
    
    # Determine if change is improvement based on configuration
    is_improvement = (change < 0) if config.improvement_means_lower else (change > 0)
    
    color = Colors.GREEN if is_improvement else Colors.RED
    direction = "↓" if change < 0 else "↑"
    
    if abs(change) == float('inf'):
        return f"{color}new benchmark{Colors.END}"
    
    return f"{color}{direction} {abs(change):.1f}%{Colors.END}"

def format_benchmark_name(name: str, config: ComparisonConfig) -> str:
    """Format benchmark name according to configuration."""
    display_name = name
    if config.name_prefix_to_remove and name.startswith(config.name_prefix_to_remove):
        display_name = name[len(config.name_prefix_to_remove):]
    display_name = display_name.replace('_', config.name_separator_replacement)
    return display_name

def compare_benchmarks(old_results: Dict, new_results: Dict, config: ComparisonConfig) -> None:
    """Compare two benchmark result sets and display the comparison."""
    
    old_benchmarks = {b['name']: b for b in old_results.get('benchmarks', [])}
    new_benchmarks = {b['name']: b for b in new_results.get('benchmarks', [])}
    
    # Get file creation times
    old_date = old_results.get('context', {}).get('date', 'Unknown')
    new_date = new_results.get('context', {}).get('date', 'Unknown')
    
    print(f"{Colors.BOLD}Benchmark Comparison Report{Colors.END}")
    print("=" * 60)
    print(f"Old results: {old_date}")
    print(f"New results: {new_date}")
    print()
    
    # Find common benchmarks
    common_benchmarks = set(old_benchmarks.keys()) & set(new_benchmarks.keys())
    new_benchmarks_only = set(new_benchmarks.keys()) - set(old_benchmarks.keys())
    removed_benchmarks = set(old_benchmarks.keys()) - set(new_benchmarks.keys())
    
    if not common_benchmarks and not new_benchmarks_only:
        print(f"{Colors.RED}No benchmarks to compare!{Colors.END}")
        return
    
    # Compare common benchmarks
    if common_benchmarks:
        print(f"{Colors.BOLD}Performance Changes:{Colors.END}")
        print()
        
        improvements = []
        regressions = []
        no_changes = []
        
        for name in sorted(common_benchmarks):
            old_bench = old_benchmarks[name]
            new_bench = new_benchmarks[name]
            
            old_time = old_bench.get('real_time', old_bench.get('cpu_time', 0))
            new_time = new_bench.get('real_time', new_bench.get('cpu_time', 0))
            time_unit = new_bench.get('time_unit', 'ns')
            
            change = calculate_change_percentage(old_time, new_time)
            
            # Format benchmark name
            display_name = format_benchmark_name(name, config)
            
            print(f"  {Colors.CYAN}{display_name:<40}{Colors.END}", end="")
            print(f" {format_number(old_time):>10} → {format_number(new_time):<10} {time_unit:<3}", end="")
            print(f" {format_percentage_change(change, config)}")
            
            # Categorize changes
            if abs(change) < config.significant_change_threshold:
                no_changes.append(name)
            elif (change < 0) if config.improvement_means_lower else (change > 0):
                improvements.append((name, abs(change)))
            else:
                regressions.append((name, abs(change) if change < 0 else change))
        
        print()
        
        # Summary
        print(f"{Colors.BOLD}Summary:{Colors.END}")
        if improvements:
            print(f"  {Colors.GREEN}✓ {len(improvements)} improvements{Colors.END}")
            for name, change in sorted(improvements, key=lambda x: x[1], reverse=True)[:config.max_summary_items]:
                display_name = format_benchmark_name(name, config)
                improvement_word = "faster" if config.improvement_means_lower else "better"
                print(f"    • {display_name}: {change:.1f}% {improvement_word}")
        
        if regressions:
            print(f"  {Colors.RED}✗ {len(regressions)} regressions{Colors.END}")
            for name, change in sorted(regressions, key=lambda x: x[1], reverse=True)[:config.max_summary_items]:
                display_name = format_benchmark_name(name, config)
                regression_word = "slower" if config.improvement_means_lower else "worse"
                print(f"    • {display_name}: {change:.1f}% {regression_word}")
        
        if no_changes:
            print(f"  {Colors.WHITE}≈ {len(no_changes)} unchanged{Colors.END}")
    
    # Show new benchmarks
    if new_benchmarks_only:
        print()
        print(f"{Colors.BOLD}New Benchmarks:{Colors.END}")
        for name in sorted(new_benchmarks_only):
            bench = new_benchmarks[name]
            time = bench.get('real_time', bench.get('cpu_time', 0))
            time_unit = bench.get('time_unit', 'ns')
            display_name = format_benchmark_name(name, config)
            print(f"  {Colors.GREEN}+ {display_name}: {format_number(time)} {time_unit}{Colors.END}")
    
    # Show removed benchmarks
    if removed_benchmarks:
        print()
        print(f"{Colors.BOLD}Removed Benchmarks:{Colors.END}")
        for name in sorted(removed_benchmarks):
            display_name = format_benchmark_name(name, config)
            print(f"  {Colors.RED}- {display_name}{Colors.END}")

def analyze_single_result(results: Dict, config: ComparisonConfig) -> None:
    """Analyze a single benchmark result file."""
    benchmarks = results.get('benchmarks', [])
    context = results.get('context', {})
    
    print(f"{Colors.BOLD}Benchmark Analysis Report{Colors.END}")
    print("=" * 50)
    
    # Show context information
    if context:
        print(f"{Colors.CYAN}Context Information:{Colors.END}")
        for key, value in context.items():
            if key in ['date', 'host_name', 'executable', 'num_cpus', 'mhz_per_cpu']:
                print(f"  {key}: {value}")
        print()
    
    # Show benchmark results
    print(f"{Colors.BOLD}Benchmark Results:{Colors.END}")
    print(f"Total benchmarks: {len(benchmarks)}")
    print()
    
    if benchmarks:
        # Group by time unit for better display
        by_unit = {}
        for bench in benchmarks:
            unit = bench.get('time_unit', 'ns')
            if unit not in by_unit:
                by_unit[unit] = []
            by_unit[unit].append(bench)
        
        for unit, unit_benchmarks in by_unit.items():
            print(f"{Colors.BOLD}Results in {format_time_unit(unit)}:{Colors.END}")
            
            for bench in sorted(unit_benchmarks, key=lambda x: x.get('real_time', x.get('cpu_time', 0))):
                name = format_benchmark_name(bench['name'], config)
                time = bench.get('real_time', bench.get('cpu_time', 0))
                iterations = bench.get('iterations', 'N/A')
                
                print(f"  {Colors.CYAN}{name:<40}{Colors.END}", end="")
                print(f" {format_number(time):>10} {unit:<3}", end="")
                print(f" ({iterations} iterations)")
            print()

def list_results(config: ComparisonConfig) -> None:
    """List all available benchmark result files."""
    results_path = Path(config.results_dir)
    
    if not results_path.exists():
        print(f"{Colors.RED}Results directory not found: {config.results_dir}{Colors.END}")
        print(f"Please run benchmarks first with: {config.run_command_template}")
        return
    
    print(f"{Colors.BOLD}Available Benchmark Results{Colors.END}")
    print("=" * 40)
    print(f"Directory: {results_path.absolute()}")
    print()
    
    candidates = []
    for file_path in results_path.glob('*.json'):
        timestamp = extract_timestamp_from_filename(file_path.name, config)
        if timestamp:
            candidates.append((timestamp, file_path))
    
    if not candidates:
        print(f"{Colors.YELLOW}No benchmark result files found matching pattern: {config.file_pattern}{Colors.END}")
        print(f"Please run benchmarks first with: {config.run_command_template}")
        return
    
    # Sort by timestamp (newest first)
    candidates.sort(key=lambda x: x[0], reverse=True)
    
    print(f"Found {len(candidates)} result files:")
    for i, (timestamp, file_path) in enumerate(candidates):
        age_marker = f"{Colors.GREEN}(latest){Colors.END}" if i == 0 else ""
        size = file_path.stat().st_size
        print(f"  {timestamp.strftime('%Y-%m-%d %H:%M:%S')} - {file_path.name} ({size:,} bytes) {age_marker}")

def cmd_compare(args) -> None:
    """Handle compare subcommand."""
    # Initialize configuration
    config = ComparisonConfig()
    config.from_args(args)
    
    # Disable colors if requested
    if args.no_color:
        for attr in dir(Colors):
            if not attr.startswith('_'):
                setattr(Colors, attr, '')
    
    # Determine new results file
    new_result_file = args.new_result
    if not new_result_file:
        # Try to find the most recent result file
        new_result_file = find_most_recent_result(config)
        if not new_result_file:
            print(f"{Colors.RED}No benchmark result files found in {config.results_dir}/ directory.{Colors.END}")
            print(f"Please run benchmarks first with: {config.run_command_template}")
            sys.exit(1)
        print(f"Using most recent result file: {new_result_file}")
    
    # Load new results
    new_results = load_benchmark_results(new_result_file)
    if not new_results:
        sys.exit(1)
    
    # Determine old results file
    old_result_file = args.old_result
    if not old_result_file:
        old_result_file = find_most_recent_previous_result(new_result_file, config)
        if not old_result_file:
            print(f"{Colors.YELLOW}No previous benchmark results found for comparison.{Colors.END}")
            print(f"Current result loaded: {new_result_file}")
            
            # Show current results summary
            benchmarks = new_results.get('benchmarks', [])
            print(f"\nCurrent benchmark contains {len(benchmarks)} tests:")
            for bench in benchmarks:
                name = format_benchmark_name(bench['name'], config)
                time = bench.get('real_time', bench.get('cpu_time', 0))
                time_unit = bench.get('time_unit', 'ns')
                print(f"  • {name}: {format_number(time)} {time_unit}")
            
            return
        else:
            print(f"Comparing with most recent previous result: {old_result_file}")
    
    # Load old results
    old_results = load_benchmark_results(old_result_file)
    if not old_results:
        sys.exit(1)
    
    # Perform comparison
    compare_benchmarks(old_results, new_results, config)

def cmd_analyze(args) -> None:
    """Handle analyze subcommand."""
    # Initialize configuration
    config = ComparisonConfig()
    config.from_args(args)
    
    # Disable colors if requested
    if args.no_color:
        for attr in dir(Colors):
            if not attr.startswith('_'):
                setattr(Colors, attr, '')
    
    # Determine result file
    result_file = args.result_file
    if not result_file:
        # Try to find the most recent result file
        result_file = find_most_recent_result(config)
        if not result_file:
            print(f"{Colors.RED}No benchmark result files found in {config.results_dir}/ directory.{Colors.END}")
            print(f"Please run benchmarks first with: {config.run_command_template}")
            sys.exit(1)
        print(f"Analyzing most recent result file: {result_file}")
    
    # Load results
    results = load_benchmark_results(result_file)
    if not results:
        sys.exit(1)
    
    # Perform analysis
    analyze_single_result(results, config)

def cmd_list(args) -> None:
    """Handle list subcommand."""
    # Initialize configuration
    config = ComparisonConfig()
    config.from_args(args)
    
    # Disable colors if requested
    if args.no_color:
        for attr in dir(Colors):
            if not attr.startswith('_'):
                setattr(Colors, attr, '')
    
    # List results
    list_results(config)

def main():
    # Check for backward compatibility - if arguments don't start with subcommand, assume compare
    if len(sys.argv) > 1 and sys.argv[1] not in ['compare', 'analyze', 'list'] and not sys.argv[1].startswith('-'):
        # Old style usage - assume compare command
        sys.argv.insert(1, 'compare')
    
    parser = argparse.ArgumentParser(
        description='Generic benchmark utility tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Subcommands:
  compare   Compare two benchmark result files
  analyze   Analyze single benchmark result file  
  list      List available benchmark result files

Examples:
  python3 benchmark.py compare                                    # Compare two most recent results
  python3 benchmark.py compare new.json old.json                 # Compare specific files
  python3 benchmark.py analyze results/latest.json               # Analyze single result
  python3 benchmark.py list                                      # Show available results
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Common arguments for all subcommands
    def add_common_args(parser):
        parser.add_argument('--results-dir', help='Directory to search for result files (default: results)')
        parser.add_argument('--file-pattern', help='Regex pattern to match result files with timestamp group (default: benchmark_(\\d{8}_\\d{6})\\.json)')
        parser.add_argument('--timestamp-format', help='Timestamp format for parsing (default: %%Y%%m%%d_%%H%%M%%S)')
        parser.add_argument('--no-color', action='store_true', help='Disable colored output')
        parser.add_argument('--name-prefix', help='Prefix to remove from benchmark names (default: BM_)')
        parser.add_argument('--run-command', help='Command to suggest for running benchmarks (default: ./run_benchmark.sh)')
    
    # Compare subcommand
    compare_parser = subparsers.add_parser('compare', help='Compare benchmark results')
    compare_parser.add_argument('new_result', nargs='?', help='Path to the new benchmark result JSON file (optional)')
    compare_parser.add_argument('old_result', nargs='?', help='Path to the old benchmark result JSON file (optional)')
    compare_parser.add_argument('--threshold', type=float, help='Minimum percentage change to be considered significant (default: 1.0)')
    compare_parser.add_argument('--higher-is-better', action='store_true', help='Higher values indicate better performance (default: lower is better)')
    add_common_args(compare_parser)
    
    # Analyze subcommand
    analyze_parser = subparsers.add_parser('analyze', help='Analyze single benchmark result')
    analyze_parser.add_argument('result_file', nargs='?', help='Path to the benchmark result JSON file (optional, uses most recent if not specified)')
    add_common_args(analyze_parser)
    
    # List subcommand
    list_parser = subparsers.add_parser('list', help='List available benchmark result files')
    add_common_args(list_parser)
    
    args = parser.parse_args()
    
    # Handle subcommands
    if args.command == 'compare':
        cmd_compare(args)
    elif args.command == 'analyze':
        cmd_analyze(args)
    elif args.command == 'list':
        cmd_list(args)
    else:
        parser.print_help()

if __name__ == '__main__':
    main()