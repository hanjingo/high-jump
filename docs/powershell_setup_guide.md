# PowerShell Setup Tools Usage Guide

This directory contains generic PowerShell execution policy setup tools that can be used across different projects.

## Files

- `setup_powershell_policy.bat` - Windows batch script for PowerShell policy setup
- `setup_powershell_policy.ps1` - Native PowerShell script with advanced features

## PowerShell Execution Policy Overview

Windows PowerShell has execution policies that control script execution:

- **Restricted** - No scripts allowed (Windows default)
- **AllSigned** - Only digitally signed scripts allowed
- **RemoteSigned** - Local scripts allowed, remote scripts must be signed (recommended)
- **Unrestricted** - All scripts allowed with warnings for remote scripts
- **Bypass** - All scripts allowed without restrictions

## Usage Examples

### Batch Script (`setup_powershell_policy.bat`)

#### Basic usage (sets RemoteSigned for CurrentUser):
```cmd
setup_powershell_policy.bat
```

#### Custom policy and scope:
```cmd
setup_powershell_policy.bat RemoteSigned CurrentUser
setup_powershell_policy.bat Unrestricted Process
setup_powershell_policy.bat AllSigned LocalMachine
```

#### Show help:
```cmd
setup_powershell_policy.bat help
setup_powershell_policy.bat InvalidPolicy
```

### PowerShell Script (`setup_powershell_policy.ps1`)

#### Basic usage:
```powershell
.\setup_powershell_policy.ps1
```

#### With parameters:
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Scope CurrentUser
.\setup_powershell_policy.ps1 RemoteSigned CurrentUser
```

#### Preview changes without applying:
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel Unrestricted -WhatIf
```

#### Force apply without confirmation:
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Force
```

#### Temporary policy for current session:
```powershell
.\setup_powershell_policy.ps1 -PolicyLevel Bypass -Scope Process -Force
```

#### Show help:
```powershell
.\setup_powershell_policy.ps1 -Help
```

## Scopes Explained

- **Process** - Affects only the current PowerShell session
- **CurrentUser** - Affects the current user (recommended for most cases)
- **LocalMachine** - Affects all users (requires Administrator privileges)
- **UserPolicy** - Set by Group Policy for the current user
- **MachinePolicy** - Set by Group Policy for all users

## Common Use Cases

### Development Environment Setup
```cmd
REM Allow local scripts for development
setup_powershell_policy.bat RemoteSigned CurrentUser
```

### Temporary Script Testing
```powershell
# Allow all scripts for current session only
.\setup_powershell_policy.ps1 -PolicyLevel Bypass -Scope Process -Force
```

### Corporate Environment
```cmd
REM Require all scripts to be signed (needs admin privileges)
setup_powershell_policy.bat AllSigned LocalMachine
```

### CI/CD Pipeline
```powershell
# Non-interactive setup for automation
.\setup_powershell_policy.ps1 -PolicyLevel RemoteSigned -Force
```

## Project Integration

### In your project's setup script:
```cmd
@echo off
echo Setting up PowerShell for this project...
call path\to\scripts\setup_powershell_policy.bat RemoteSigned CurrentUser
echo You can now run PowerShell scripts in this project.
```

### In PowerShell project setup:
```powershell
# Check and setup PowerShell policy
if ((Get-ExecutionPolicy -Scope CurrentUser) -eq 'Restricted') {
    Write-Host "Setting up PowerShell execution policy..."
    & "path\to\scripts\setup_powershell_policy.ps1" -Force
}
```

## Troubleshooting

### "Execution policy cannot be changed" Error
- Run as Administrator for LocalMachine scope
- Check if Group Policy is preventing changes
- Try CurrentUser scope instead of LocalMachine

### Scripts still won't run after policy change
- Close and reopen PowerShell to refresh policies
- Check if antivirus is blocking scripts
- Verify the script file isn't blocked (right-click → Properties → Unblock)

### Group Policy Override
If Group Policy is controlling execution policies:
- Contact your IT administrator
- Use Process scope for temporary overrides
- Consider using signed scripts if AllSigned is enforced