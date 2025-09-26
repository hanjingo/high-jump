# 基准测试工具

集成的基准测试运行器，具有自动比较功能。

## 本地测试

`results/` 目录会自动创建，用于在本地测试期间存储基准测试结果。该目录被git忽略，因为基准测试结果在不同的机器、CPU架构和系统配置之间存在显著差异。结果仅用于本地性能分析和比较。

## 脚本

- `run_benchmark.sh` - Unix/Linux/macOS系统的Shell脚本，带集成比较功能
- `run_benchmark.bat` - Windows系统的批处理脚本，带集成比较功能
- `run_benchmark.ps1` - Windows系统的PowerShell脚本，带集成比较功能（推荐）
- `../scripts/benchmark.py` - 通用基准测试工具（内部使用）
- `../scripts/setup_powershell_policy.bat` - 通用PowerShell策略设置工具（批处理）
- `../scripts/setup_powershell_policy.ps1` - 通用PowerShell策略设置工具（PowerShell）

## 使用方法

### 运行基准测试

1. **首先构建项目：**
   ```bash
   # 从根目录运行 (Unix/Linux/macOS)
   cmake -B build
   cmake --build build --target benchs
   ```
   
   ```cmd
   :: 从根目录运行 (Windows 命令提示符)
   cmake -B build
   cmake --build build --target benchs
   ```
   
   ```powershell
   # 从根目录运行 (Windows PowerShell)
   cmake -B build
   cmake --build build --target benchs
   ```

2. **运行基准测试：**

   **基本基准测试运行（仅保存结果）：**
   
   **Unix/Linux/macOS：**
   ```bash
   # 在benchs目录中运行
   ./run_benchmark.sh [基准测试选项]
   ```
   
   **Windows 命令提示符：**
   ```cmd
   :: 在benchs目录中运行  
   run_benchmark.bat [基准测试选项]
   ```
   
   **Windows PowerShell：**
   ```powershell
   # 在benchs目录中运行
   .\run_benchmark.ps1 [基准测试选项]
   ```

   **带自动比较的基准测试运行：**
   
   **Unix/Linux/macOS：**
   ```bash
   # 在benchs目录中运行
   ./run_benchmark.sh --compare [基准测试选项]
   ```
   
   **Windows 命令提示符：**
   ```cmd
   :: 在benchs目录中运行  
   run_benchmark.bat /compare [基准测试选项]
   ```
   
   **Windows PowerShell：**
   ```powershell
   # 在benchs目录中运行
   .\run_benchmark.ps1 -Compare [基准测试选项]
   ```

### 示例

**基本基准测试运行：**
```bash
./run_benchmark.sh
```

**带比较运行：**
```bash
./run_benchmark.sh --compare
```

**运行特定基准测试并比较：**
```bash
./run_benchmark.sh --compare --benchmark_filter="BM_Defer.*"
```

**Windows 等效命令：**
```cmd
run_benchmark.bat /compare --benchmark_filter="BM_Defer.*"
```
```powershell
.\run_benchmark.ps1 -Compare --benchmark_filter="BM_Defer.*"
```

### 功能特性

- **时间戳结果**：自动生成基于时间戳的文件名（`benchmark_YYYYMMDD_HHMMSS.json`）
- **双重输出**：同时保存JSON格式（用于比较）和文本格式（便于阅读）
- **跨平台**：在Linux、macOS和Windows上功能一致
- **集成比较**：可选标志，可在基准测试后立即运行比较
- **自动检测**：自动查找并与最近的历史结果进行比较
- **错误处理**：清晰的错误信息和有用的建议
- **文件大小报告**：显示结果文件大小以供参考

### 工作流程

1. **首次运行**：`./run_benchmark.sh` - 创建初始基线
2. **进行更改**：修改代码、设置或环境
3. **比较运行**：`./run_benchmark.sh --compare` - 运行基准测试并显示比较结果
4. **迭代**：重复步骤2-3进行持续性能监控

## 手动比较

如果您更喜欢手动控制比较，可以直接使用通用基准测试工具：

```bash
python3 ../scripts/benchmark.py compare \
    --results-dir=results \
    --file-pattern='benchmark_(\d{8}_\d{6})\.json' \
    --timestamp-format='%Y%m%d_%H%M%S' \
    --name-prefix='BM_' \
    --threshold=1.0 \
    --run-command='./run_benchmark.sh'
```

您还可以使用其他子命令：

```bash
# 分析单个结果文件
python3 ../scripts/benchmark.py analyze results/benchmark_20250927_143022.json

# 列出所有可用的结果文件
python3 ../scripts/benchmark.py list

# 获取任何子命令的帮助信息
python3 ../scripts/benchmark.py compare --help
```

## 输出示例

### 基本运行输出

```
Running benchmarks...
Timestamp: 20250927_143022
Results will be saved to:
  JSON: results/benchmark_20250927_143022.json
  Text: results/benchmark_20250927_143022.txt

[基准测试执行...]

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

### 比较运行输出

```
Running benchmarks...
Timestamp: 20250927_143022
[基准测试执行...]

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

## CMake 目标

CMakeLists.txt 还提供了这些目标：

- `cmake --build build --target benchmark_console` - 仅在控制台输出运行
- `cmake --build build --target benchmark_run` - 使用JSON导出运行（需要TIMESTAMP环境变量）

## 系统要求

- C++17 编译器
- Google Benchmark 库
- Python 3.6+（用于比较脚本）
- **Unix/Linux/macOS：** Bash shell
- **Windows：** 命令提示符或PowerShell（推荐PowerShell以获得更好的Unicode支持）

## 文件命名约定

结果文件自动使用时间戳命名：
- 格式：`benchmark_YYYYMMDD_HHMMSS.json`
- 示例：`benchmark_20250927_143022.json`

这确保了按时间顺序排列并防止文件冲突。

## Windows特定说明

### PowerShell 执行策略
如果在运行 `run_benchmark.ps1` 时遇到执行策略错误，您可以选择：

1. **使用通用PowerShell设置工具：**
   ```cmd
   ..\scripts\setup_powershell_policy.bat
   ```
   ```powershell
   ..\scripts\setup_powershell_policy.ps1
   ```

2. **手动设置执行策略：**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

3. **改用批处理脚本：**
   ```cmd
   run_benchmark.bat
   ```

### 路径分隔符
- Python比较脚本自动处理Unix（`/`）和Windows（`\`）路径分隔符
- 您可以在指定文件路径时使用任一格式

### Unicode支持
- PowerShell脚本为基准测试名称和输出提供更好的Unicode支持
- 批处理脚本在某些控制台配置中可能对特殊字符有问题