# 代码格式化脚本

本目录包含用于high-jump C++项目的跨平台代码格式化脚本，使用clang-format进行代码格式化。

## 概述

格式化脚本为不同平台提供统一的代码格式化接口：
- `format.sh` - 适用于Linux和macOS的跨平台Bash脚本
- `format.ps1` - 适用于Windows的PowerShell脚本

## 前置条件

### 所有平台
- **clang-format** 必须已安装并在PATH中可用
- 必须从项目根目录（包含`.clang-format`文件）执行

### 安装说明

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
# 使用Homebrew
brew install clang-format

# 或安装Xcode命令行工具
xcode-select --install
```

#### Windows
```powershell
# 使用Chocolatey
choco install llvm

# 使用Scoop
scoop install llvm

# 或从LLVM发布页面下载
# https://github.com/llvm/llvm-project/releases
```

## 使用方法

### Linux/macOS (format.sh)

```bash
# 格式化所有C++文件
./scripts/format.sh

# 仅检查格式化而不做更改
./scripts/format.sh --check
./scripts/format.sh -c

# 启用详细输出
./scripts/format.sh --verbose
./scripts/format.sh -v

# 组合选项
./scripts/format.sh --check --verbose

# 格式化指定文件
./scripts/format.sh file1.hpp file2.cpp

# 详细检查指定文件
./scripts/format.sh --check --verbose hj/net/tcp/tcp_socket.hpp
```

### Windows (format.ps1)

```powershell
# 格式化所有C++文件
./scripts/format.ps1

# 仅检查格式化而不做更改
./scripts/format.ps1 -Check
```

## 选项说明

### format.sh 选项

| 选项 | 简写 | 描述 |
|------|------|------|
| `--check` | `-c` | 仅检查格式化而不做更改 |
| `--verbose` | `-v` | 启用详细输出显示详细信息 |
| `--help` | `-h` | 显示帮助信息和用法说明 |

### format.sh 参数

| 参数 | 描述 |
|------|------|
| `FILES...` | 指定要格式化的文件（可选，默认处理所有C++文件） |

### format.ps1 选项

| 参数 | 描述 |
|------|------|
| `-Check` | 仅检查格式化而不做更改 |

## 文件覆盖范围

两个脚本都会处理以下目录中的C++源文件：
- `hj/` - 头文件和实现文件
- `tests/` - 测试文件

### 支持的文件扩展名
- `.cpp` - C++源文件
- `.hpp` - C++头文件
- `.h` - C头文件
- `.cc` - C++源文件（替代扩展名）
- `.cxx` - C++源文件（替代扩展名）

## 行为模式

### 格式化模式（默认）
- 扫描目标目录中的C++文件
- 对每个文件就地应用clang-format
- 报告哪些文件被修改
- 显示格式化文件的总数

### 检查模式
- 验证格式化而不做更改
- 报告需要格式化的文件
- 如果发现格式化问题则以错误代码退出
- 适用于CI/CD流水线和提交前钩子

## 输出示例

### 成功格式化
```bash
$ ./scripts/format.sh
Found 42 C++ files
Applying code formatting...
Formatted: astar.hpp
Formatted: matrix.hpp
Code formatting complete! 2 files were modified.
```

### 检查模式 - 发现问题
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

### 详细检查指定文件示例
```bash
# 详细检查单个文件格式
$ ./scripts/format.sh --check --verbose hj/net/tcp/tcp_socket.hpp
Project root: /usr/local/src/github.com/hanjingo/high-jump
Using: clang-format version 21.1.2
Processing specified files...
Found 1 C++ files
Files to process:
  hj/net/tcp/tcp_socket.hpp
Checking code formatting...
Checking: hj/net/tcp/tcp_socket.hpp
All files are properly formatted!

# 检查多个指定文件
$ ./scripts/format.sh --check file1.hpp file2.cpp
Found 2 C++ files
Checking code formatting...
All files are properly formatted!
```

## 集成方式

### 提交前钩子
添加到`.git/hooks/pre-commit`：
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

### VS Code 任务
添加到`.vscode/tasks.json`：
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

## 配置

格式化行为由项目根目录中的`.clang-format`文件控制。这确保了所有团队成员和环境中的一致格式化规则。

## 错误处理

### 常见问题

1. **找不到clang-format**
   - 按照特定平台说明安装clang-format
   - 确保它在PATH中可用

2. **不在项目根目录**
   - 导航到包含`.clang-format`的目录
   - 脚本会验证配置文件的存在

3. **权限被拒绝**
   - 确保脚本有执行权限：
     ```bash
     chmod +x scripts/format.sh
     ```

4. **找不到文件**
   - 验证`hj/`和`tests/`目录是否存在
   - 检查目录是否包含C++文件

### 退出代码

| 代码 | 含义 |
|------|------|
| 0 | 成功 - 所有文件格式正确 |
| 1 | 错误 - 发现格式化问题或工具失败 |

## 最佳实践

1. **提交前运行**：创建提交前始终格式化代码
2. **在CI中使用检查模式**：在持续集成中验证格式化
3. **团队一致性**：确保所有团队成员使用相同的clang-format版本
4. **IDE集成**：配置您的IDE使用相同的格式化规则

## 故障排除

### 详细模式
使用详细模式调试问题：
```bash
./scripts/format.sh --check --verbose
```

这提供了以下详细信息：
- 项目根目录检测
- clang-format版本
- 正在处理的文件
- 详细错误消息

### 手动格式化
对于单个文件：
```bash
clang-format --style=file -i path/to/file.cpp
```

### 版本兼容性
不同的clang-format版本可能产生略有不同的结果。为了团队一致性，请在项目文档中记录推荐的版本。

## 技术实现

### 脚本特性

#### format.sh
- **跨平台兼容性**：支持Linux和macOS
- **颜色输出**：使用ANSI颜色代码提供视觉反馈
- **错误处理**：完善的错误检查和用户友好的错误消息
- **参数解析**：支持长短选项格式
- **详细日志**：可选的详细输出用于调试

#### format.ps1
- **PowerShell原生**：使用PowerShell最佳实践
- **参数验证**：内置的参数类型检查
- **错误处理**：PowerShell异常处理机制
- **跨平台PowerShell**：支持PowerShell Core

### 安全性考虑
- 所有脚本使用`set -euo pipefail`（Bash）确保错误传播
- 输入验证防止意外的文件修改
- 路径解析防止目录遍历攻击

### 性能优化
- 使用`find`命令高效查找文件
- 批处理文件操作减少I/O开销
- 临时文件管理确保内存效率