# PowerShell 设置工具使用指南

此目录包含可在不同项目中使用的通用PowerShell执行策略设置工具。

## 文件

- `setup_powershell_policy.bat` - PowerShell策略设置的Windows批处理脚本
- `setup_powershell_policy.ps1` - 具有高级功能的原生PowerShell脚本

## PowerShell 执行策略概述

Windows PowerShell有控制脚本执行的执行策略：

- **Restricted** - 不允许脚本（Windows默认）
- **AllSigned** - 仅允许数字签名的脚本
- **RemoteSigned** - 允许本地脚本，远程脚本必须签名（推荐）
- **Unrestricted** - 允许所有脚本并对远程脚本发出警告
- **Bypass** - 允许所有脚本且无限制

## 使用示例

### 批处理脚本 (`setup_powershell_policy.bat`)

#### 基本用法（为CurrentUser设置RemoteSigned）：
```cmd
setup_powershell_policy.bat
```

#### 自定义策略和范围：
```cmd
setup_powershell_policy.bat RemoteSigned CurrentUser
setup_powershell_policy.bat Unrestricted Process
setup_powershell_policy.bat AllSigned LocalMachine
```

#### 显示帮助：
```cmd
setup_powershell_policy.bat help
setup_powershell_policy.bat InvalidPolicy
```

### PowerShell 脚本 (`setup_powershell_policy.ps1`)

#### 基本用法：
```powershell
.\setup_powershell_policy.ps1
```

#### 带参数：
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Scope CurrentUser
.\setup_powershell_policy.ps1 RemoteSigned CurrentUser
```

#### 预览更改而不应用：
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel Unrestricted -WhatIf
```

#### 强制应用而不确认：
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Force
```

#### 当前会话的临时策略：
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel Bypass -Scope Process -Force
```

#### 显示帮助：
```powershell
.\setup_powershell_policy.ps1 -Help
```

## 范围说明

- **Process** - 仅影响当前PowerShell会话
- **CurrentUser** - 影响当前用户（大多数情况下推荐）
- **LocalMachine** - 影响所有用户（需要管理员权限）
- **UserPolicy** - 由组策略为当前用户设置
- **MachinePolicy** - 由组策略为所有用户设置

## 常见用例

### 开发环境设置
```cmd
REM 允许本地脚本用于开发
setup_powershell_policy.bat RemoteSigned CurrentUser
```

### 临时脚本测试
```powershell
# 仅在当前会话中允许所有脚本
.\setup_powershell_policy.ps1 -PolicyLevel Bypass -Scope Process -Force
```

### 企业环境
```cmd
REM 要求所有脚本必须签名（需要管理员权限）
setup_powershell_policy.bat AllSigned LocalMachine
```

### CI/CD 管道
```powershell
# 用于自动化的非交互式设置
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Force
```

## 项目集成

### 在项目的设置脚本中：
```cmd
@echo off
echo Setting up PowerShell for this project...
call path\to\scripts\setup_powershell_policy.bat RemoteSigned CurrentUser
echo You can now run PowerShell scripts in this project.
```

### 在PowerShell项目设置中：
```powershell
# 检查并设置PowerShell策略
if ((Get-ExecutionPolicy -Scope CurrentUser) -eq 'Restricted') {
    Write-Host "Setting up PowerShell execution policy..."
    & "path\to\scripts\setup_powershell_policy.ps1" -Force
}
```

## 故障排除

### "执行策略无法更改"错误
- 对LocalMachine范围以管理员身份运行
- 检查组策略是否阻止更改
- 尝试CurrentUser范围而不是LocalMachine

### 策略更改后脚本仍无法运行
- 关闭并重新打开PowerShell以刷新策略
- 检查防病毒软件是否阻止脚本
- 验证脚本文件未被阻止（右键单击→属性→解除阻止）

### 组策略覆盖
如果组策略控制执行策略：
- 联系您的IT管理员
- 使用Process范围进行临时覆盖
- 如果强制执行AllSigned，考虑使用签名脚本