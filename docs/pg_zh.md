# PostgreSQL 安装脚本文档

用于跨平台部署的通用PostgreSQL安装和配置脚本。

## 概述

`pg.sh` 脚本提供了在各种操作系统上自动安装和配置PostgreSQL的方法。它支持多个PostgreSQL版本、自定义配置和灵活的部署选项。

## 功能特性

- **跨平台支持**: Linux (Ubuntu/Debian, CentOS/RHEL/Fedora) 和 macOS
- **版本灵活性**: 安装指定PostgreSQL版本或最新稳定版
- **自定义配置**: 可配置数据库名、用户、密码和端口
- **安装控制**: 支持跳过初始化或服务启动的选项
- **试运行模式**: 预览安装步骤而不执行更改
- **详细日志**: 安装跟踪的详细输出

## 用法

### 基本语法

```bash
./scripts/pg.sh [选项]
```

### 快速开始

```bash
# 使用默认设置安装PostgreSQL
./scripts/pg.sh

# 安装指定版本并使用自定义用户
./scripts/pg.sh -v 15 -u myuser -p mypassword

# 预览安装过程而不执行
./scripts/pg.sh --dry-run
```

## 选项

| 选项 | 简写 | 描述 | 默认值 |
|------|------|------|--------|
| `--version VERSION` | `-v` | 要安装的PostgreSQL版本 | `latest` |
| `--dest DIR` | `-d` | 安装目标目录 | 系统默认 |
| `--user USERNAME` | `-u` | 要创建的数据库用户 | `pguser` |
| `--password PASSWORD` | `-p` | 数据库用户密码 | `pgpass` |
| `--database DBNAME` | | 要创建的数据库名 | `postgres` |
| `--port PORT` | | PostgreSQL端口 | `5432` |
| `--help` | `-h` | 显示帮助信息 | |
| `--dry-run` | | 显示将要执行的操作而不实际执行 | |
| `--no-init` | | 跳过数据库初始化 | |
| `--no-start` | | 跳过启动PostgreSQL服务 | |

## 示例

### 版本特定安装

```bash
# 安装PostgreSQL 15
./scripts/pg.sh -v 15

# 安装PostgreSQL 14并使用自定义配置
./scripts/pg.sh -v 14 -u appuser -p apppass --database myapp
```

### 自定义配置

```bash
# 自定义用户和数据库
./scripts/pg.sh -u myuser -p mypass --database mydb

# 自定义端口并跳过自动启动
./scripts/pg.sh --port 5433 --no-start

# 仅安装，跳过初始化
./scripts/pg.sh --no-init
```

### 开发工作流

```bash
# 预览安装
./scripts/pg.sh --dry-run

# 开发环境安装
./scripts/pg.sh -u devuser -p devpass --database devdb --port 5433

# 指定版本的生产安装
./scripts/pg.sh -v 15 -u produser -p $(cat /secure/pgpass) --database proddb
```

## 平台支持

### Linux

#### Ubuntu/Debian
- 使用 `apt` 包管理器
- 通过PostgreSQL官方仓库支持特定版本
- 需要 `sudo` 权限

#### CentOS/RHEL/Fedora/Rocky/AlmaLinux
- 使用 `yum` 或 `dnf` 包管理器
- 支持PostgreSQL官方仓库的特定版本
- 需要 `sudo` 权限

### macOS
- 使用Homebrew包管理器
- 需要先安装Homebrew
- 版本支持有限（取决于Homebrew公式）

## 安装过程

### 1. 包安装
- 安装PostgreSQL服务器和contrib包
- 在需要时添加官方仓库以获取特定版本
- 处理平台特定的包管理器差异

### 2. 服务配置
- 启用PostgreSQL服务（除非指定 `--no-start`）
- 启动PostgreSQL服务
- 配置系统服务管理

### 3. 数据库初始化
- 设置初始数据库集群（仅Linux，除非指定 `--no-init`）
- 配置postgres超级用户密码
- 使用指定凭据创建自定义数据库和用户
- 授予适当权限

## 安全考虑

### 密码处理
```bash
# 使用环境变量处理敏感数据
export PG_PASS="your-secure-password"
./scripts/pg.sh -p "$PG_PASS"

# 从安全文件读取
./scripts/pg.sh -p "$(cat /secure/postgres.pass)"
```

### 用户权限
- 创建的用户具有 `CREATEDB` 权限
- 对指定数据库授予完全权限
- 生产环境建议遵循最小权限原则

## 故障排除

### 常见问题

**权限被拒绝错误**
```bash
# 确保脚本可执行
chmod +x scripts/pg.sh

# 在Linux上使用适当权限运行
sudo ./scripts/pg.sh
```

**服务未启动**
```bash
# 检查服务状态 (Linux)
sudo systemctl status postgresql

# 检查日志 (Linux)
sudo journalctl -u postgresql

# 检查Homebrew服务 (macOS)
brew services list | grep postgresql
```

**连接问题**
```bash
# 测试连接
psql -h localhost -p 5432 -U pguser -d postgres

# 检查端口可用性
netstat -tulpn | grep :5432
```

### 验证

```bash
# 检查PostgreSQL版本
psql --version

# 测试数据库连接
psql -h localhost -U pguser -d postgres -c "SELECT version();"

# 列出数据库
psql -h localhost -U pguser -c "\\l"
```

## 集成示例

### Docker环境

```bash
# 为容器化开发安装PostgreSQL
./scripts/pg.sh -u dockeruser -p dockerpass --database containerdb --port 5434
```

### CI/CD管道

```bash
# 自动化测试设置
./scripts/pg.sh --dry-run  # 验证配置
./scripts/pg.sh -u testuser -p testpass --database testdb --no-start
sudo systemctl start postgresql
```

### 开发环境

```bash
# 本地开发设置
./scripts/pg.sh -v 15 -u devuser -p devpass --database myapp_dev --port 5432
```

## 高级用法

### 自定义安装路径

```bash
# 安装到自定义目录（支持时）
./scripts/pg.sh -d /opt/postgresql
```

### 多版本

```bash
# 安装不同版本进行测试
./scripts/pg.sh -v 14 --port 5434 --no-start
./scripts/pg.sh -v 15 --port 5435 --no-start
```

## 要求

### 系统要求
- Linux: Ubuntu 18.04+, Debian 10+, CentOS 7+, RHEL 7+, Fedora 30+
- macOS: 10.14+ 并安装Homebrew
- 足够的磁盘空间（因版本而异，基础安装通常需要50MB+）

### 权限
- Linux: 系统包安装需要 `sudo` 权限
- macOS: 需要Homebrew写权限

### 依赖项
- `wget` 或 `curl` 用于下载包（Linux）
- 互联网连接用于包下载

## 注意事项

- 脚本自动检测操作系统并使用适当的包管理器
- 在Linux上，PostgreSQL数据目录位置因发行版而异
- 通过Homebrew在macOS上安装可能有不同的默认配置
- 始终先在非生产环境中测试
- 生产部署时考虑防火墙和网络安全设置

## 相关文档

- [PostgreSQL官方文档](https://www.postgresql.org/docs/)
- [Homebrew PostgreSQL公式](https://formulae.brew.sh/formula/postgresql)
- [PostgreSQL APT仓库](https://wiki.postgresql.org/wiki/Apt)
- [PostgreSQL YUM仓库](https://yum.postgresql.org/)