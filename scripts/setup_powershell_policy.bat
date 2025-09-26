@echo off
REM Generic PowerShell Execution Policy Setup Tool
REM 
REM This script helps enable PowerShell script execution for the current user.
REM It sets the execution policy to RemoteSigned, which allows locally created
REM scripts to run while requiring downloaded scripts to be signed.
REM
REM Usage: setup_powershell_policy.bat [policy_level] [scope]
REM
REM Arguments:
REM   policy_level: ExecutionPolicy level (default: RemoteSigned)
REM                 Options: Restricted, AllSigned, RemoteSigned, Unrestricted, Bypass
REM   scope:        Scope to apply policy (default: CurrentUser)
REM                 Options: Process, CurrentUser, LocalMachine
REM
REM Examples:
REM   setup_powershell_policy.bat
REM   setup_powershell_policy.bat RemoteSigned CurrentUser
REM   setup_powershell_policy.bat Unrestricted Process

setlocal enabledelayedexpansion

REM Parse command line arguments or use defaults
set "POLICY_LEVEL=%~1"
set "POLICY_SCOPE=%~2"

if "%POLICY_LEVEL%"=="" set "POLICY_LEVEL=RemoteSigned"
if "%POLICY_SCOPE%"=="" set "POLICY_SCOPE=CurrentUser"

echo PowerShell Execution Policy Setup Tool
echo =====================================
echo.
echo Current settings:
echo   Policy Level: %POLICY_LEVEL%
echo   Scope:        %POLICY_SCOPE%
echo.

REM Validate policy level
set "VALID_POLICIES=Restricted AllSigned RemoteSigned Unrestricted Bypass"
set "POLICY_VALID=false"
for %%p in (%VALID_POLICIES%) do (
    if /i "%POLICY_LEVEL%"=="%%p" set "POLICY_VALID=true"
)

if "%POLICY_VALID%"=="false" (
    echo ✗ Error: Invalid policy level '%POLICY_LEVEL%'
    echo Valid options: %VALID_POLICIES%
    echo.
    goto :show_help
)

REM Validate scope
set "VALID_SCOPES=Process CurrentUser LocalMachine"
set "SCOPE_VALID=false"
for %%s in (%VALID_SCOPES%) do (
    if /i "%POLICY_SCOPE%"=="%%s" set "SCOPE_VALID=true"
)

if "%SCOPE_VALID%"=="false" (
    echo ✗ Error: Invalid scope '%POLICY_SCOPE%'
    echo Valid options: %VALID_SCOPES%
    echo.
    goto :show_help
)

REM Check current execution policy
echo Checking current PowerShell execution policy...
for /f "tokens=*" %%i in ('powershell -Command "Get-ExecutionPolicy -Scope %POLICY_SCOPE%"') do set "CURRENT_POLICY=%%i"
echo Current policy for %POLICY_SCOPE%: %CURRENT_POLICY%
echo.

REM Check if change is needed
if /i "%CURRENT_POLICY%"=="%POLICY_LEVEL%" (
    echo ✓ PowerShell execution policy is already set to %POLICY_LEVEL% for %POLICY_SCOPE%.
    echo No changes needed.
    goto :end
)

REM Apply new execution policy
echo Setting PowerShell execution policy to %POLICY_LEVEL% for %POLICY_SCOPE%...
powershell -Command "Set-ExecutionPolicy -ExecutionPolicy %POLICY_LEVEL% -Scope %POLICY_SCOPE% -Force" 2>nul

REM Check result
if %errorlevel% equ 0 (
    echo ✓ PowerShell execution policy updated successfully!
    echo.
    echo New settings:
    for /f "tokens=*" %%i in ('powershell -Command "Get-ExecutionPolicy -Scope %POLICY_SCOPE%"') do echo   %POLICY_SCOPE%: %%i
    echo.
    echo You can now run PowerShell scripts in this scope.
) else (
    echo ✗ Failed to update PowerShell execution policy.
    echo.
    echo Possible solutions:
    if /i "%POLICY_SCOPE%"=="LocalMachine" (
        echo • Run this script as Administrator for LocalMachine scope
    )
    echo • Try a different scope (Process, CurrentUser, LocalMachine)
    echo • Check if PowerShell is properly installed
    echo • Use a less restrictive policy level
    echo.
    echo Current execution policies:
    powershell -Command "Get-ExecutionPolicy -List" 2>nul
)

goto :end

:show_help
echo Usage: %~nx0 [policy_level] [scope]
echo.
echo Policy Levels:
echo   Restricted    - No scripts allowed (Windows default)
echo   AllSigned     - Only signed scripts allowed
echo   RemoteSigned  - Local scripts allowed, remote scripts must be signed (recommended)
echo   Unrestricted  - All scripts allowed with warning for remote scripts
echo   Bypass        - All scripts allowed without warnings
echo.
echo Scopes:
echo   Process       - Affects only current PowerShell session
echo   CurrentUser   - Affects current user (recommended)
echo   LocalMachine  - Affects all users (requires Administrator)
echo.
echo Examples:
echo   %~nx0                                    # Set RemoteSigned for CurrentUser
echo   %~nx0 RemoteSigned CurrentUser          # Explicit default settings
echo   %~nx0 Unrestricted Process              # Temporary unrestricted for current session
echo   %~nx0 AllSigned LocalMachine            # Require signatures for all users

:end
echo.
pause
endlocal