# 基准测试工具（benchmark.py）

一个多用途的基准测试工具，提供多种子命令用于基准结果分析。

## 概述

`benchmark.py` 脚本已重构为多功能基准测试工具，支持以下子命令：

- **compare**：比较两个基准测试结果文件（默认行为，兼容旧版）
- **analyze**：分析单个基准测试结果文件，输出详细统计
- **list**：列出 results 目录下所有可用的基准测试结果文件

## 用法

### compare 子命令（默认）

比较两个基准测试结果文件：

```bash
# 自动比较最新结果
python3 benchmark.py compare

# 比较指定文件
python3 benchmark.py compare new.json old.json

# 使用自定义参数比较
python3 benchmark.py compare --threshold=2.0 --higher-is-better
```

### analyze 子命令

分析单个基准测试结果文件：

```bash
# 分析最新结果文件
python3 benchmark.py analyze

# 分析指定文件
python3 benchmark.py analyze results/benchmark_20250927_143022.json

# 使用自定义显示选项分析
python3 benchmark.py analyze --no-color --name-prefix="Test_"
```

### list 子命令

列出所有可用的基准测试结果文件：

```bash
# 列出默认 results 目录下的文件
python3 benchmark.py list

# 列出自定义目录下的文件
python3 benchmark.py list --results-dir=output

# 使用自定义模式列出文件
python3 benchmark.py list --file-pattern="test_(\d+)\.json"
```

## 兼容旧版

工具保持对旧版 `compare_benchmark_results.py` 的兼容：

```bash
# 旧用法（仍可用）
python3 benchmark.py --results-dir=results --threshold=1.5

# 新推荐用法
python3 benchmark.py compare --results-dir=results --threshold=1.5
```

## 集成说明

该工具可被 `run_benchmark` 脚本自动调用（带 --compare 参数）：

```bash
# Shell 脚本调用：python3 ../scripts/benchmark.py compare [options]
./run_benchmark.sh --compare

# 批处理脚本调用：python ..\scripts\benchmark.py compare [options]  
run_benchmark.bat /compare

# PowerShell 脚本调用：python ..\scripts\benchmark.py compare [options]
.\run_benchmark.ps1 -Compare
```

## 可用选项

### 通用选项（所有子命令均支持）

- `--results-dir`：指定结果文件目录
- `--file-pattern`：结果文件匹配的正则表达式
- `--timestamp-format`：从文件名解析时间戳的格式
- `--no-color`：禁用彩色输出
- `--name-prefix`：去除基准名称前缀
- `--run-command`：建议的基准运行命令

### compare 专用选项

- `--threshold`：判定为显著变化的最小百分比
- `--higher-is-better`：高数值表示性能更好

## 示例

### 开发流程

```bash
# 运行基准测试
./run_benchmark.sh

# 列出所有结果
python3 ../scripts/benchmark.py list

# 分析最新结果
python3 ../scripts/benchmark.py analyze

# 代码变更后带对比运行
./run_benchmark.sh --compare
```

### 手动分析

```bash
# 比较指定版本
python3 benchmark.py compare \
    results/benchmark_20250927_143022.json \
    results/benchmark_20250927_142015.json \
    --threshold=0.5

# 使用自定义参数分析
python3 benchmark.py analyze \
    results/latest.json \
    --name-prefix="BM_" \
    --no-color
```

### CI/CD 集成

```bash
# 日志输出所有结果文件
python3 benchmark.py list --no-color > benchmark_files.log

# 使用指定参数进行结果对比
python3 benchmark.py compare \
    --threshold=5.0 \
    --higher-is-better \
    --no-color \
    > comparison_report.txt
```
