# Generic PowerShell Execution Policy Setup Tool
# 
# This script helps configure PowerShell script execution policies.
# It provides a more user-friendly interface than the built-in Set-ExecutionPolicy cmdlet.
#
# Usage: .\policy.ps1 [PolicyLevel] [Scope] [-WhatIf] [-Force]
#
# Parameters:
#   PolicyLevel: ExecutionPolicy level (default: RemoteSigned)
#                Options: Restricted, AllSigned, RemoteSigned, Unrestricted, Bypass
#   Scope:       Scope to apply policy (default: CurrentUser)
#                Options: Process, CurrentUser, LocalMachine, UserPolicy, MachinePolicy
#   WhatIf:      Show what would be changed without making changes
#   Force:       Skip confirmation prompts
#
# Examples:
#   .\policy.ps1
#   .\policy.ps1 -PolicyLevel RemoteSigned -Scope CurrentUser
#   .\policy.ps1 -PolicyLevel Unrestricted -Scope Process -Force
#   .\policy.ps1 -WhatIf

param(
    [Parameter(Position = 0)]
    [ValidateSet('Restricted', 'AllSigned', 'RemoteSigned', 'Unrestricted', 'Bypass')]
    [string]$PolicyLevel = 'RemoteSigned',
    
    [Parameter(Position = 1)]
    [ValidateSet('Process', 'CurrentUser', 'LocalMachine', 'UserPolicy', 'MachinePolicy')]
    [string]$Scope = 'CurrentUser',
    
    [switch]$WhatIf,
    [switch]$Force,
    [switch]$Help
)

# Color output functions
function Write-Success { param($Message) Write-Host "✓ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "✗ $Message" -ForegroundColor Red }
function Write-Warning { param($Message) Write-Host "⚠ $Message" -ForegroundColor Yellow }
function Write-Info { param($Message) Write-Host "ℹ $Message" -ForegroundColor Cyan }

function Show-Help {
    Write-Host @"
PowerShell Execution Policy Setup Tool
======================================

This tool helps configure PowerShell script execution policies with a user-friendly interface.

USAGE:
    .\policy.ps1 [PolicyLevel] [Scope] [-WhatIf] [-Force] [-Help]

PARAMETERS:
    PolicyLevel    Execution policy level (default: RemoteSigned)
    Scope          Scope to apply the policy (default: CurrentUser)
    -WhatIf        Show what would be changed without making changes
    -Force         Skip confirmation prompts
    -Help          Show this help message

POLICY LEVELS:
    Restricted     No scripts allowed (Windows default)
    AllSigned      Only digitally signed scripts allowed
    RemoteSigned   Local scripts allowed, remote scripts must be signed (recommended)
    Unrestricted   All scripts allowed with warning for remote scripts
    Bypass         All scripts allowed without warnings or blocking

SCOPES:
    Process        Affects only the current PowerShell session
    CurrentUser    Affects the current user (recommended for most cases)
    LocalMachine   Affects all users (requires Administrator privileges)
    UserPolicy     Set by Group Policy for the current user
    MachinePolicy  Set by Group Policy for all users

EXAMPLES:
    # Set RemoteSigned policy for current user (recommended)
    .\policy.ps1

    # Set specific policy and scope
    .\policy.ps1 RemoteSigned CurrentUser

    # Temporarily allow all scripts for current session
    .\policy.ps1 Unrestricted Process -Force

    # Check what would be changed without applying
    .\policy.ps1 AllSigned LocalMachine -WhatIf

    # Set policy for all users (requires admin privileges)
    .\policy.ps1 RemoteSigned LocalMachine
"@ -ForegroundColor White
}

function Get-PolicyDescription {
    param($Policy)
    switch ($Policy) {
        'Restricted'    { 'No PowerShell scripts are allowed to run' }
        'AllSigned'     { 'Only scripts signed by a trusted publisher can run' }
        'RemoteSigned'  { 'Local scripts can run; remote scripts must be signed' }
        'Unrestricted'  { 'All scripts can run with warnings for remote scripts' }
        'Bypass'        { 'All scripts can run without restrictions or warnings' }
        default         { 'Unknown policy' }
    }
}

function Get-ScopeDescription {
    param($Scope)
    switch ($Scope) {
        'Process'       { 'Current PowerShell session only' }
        'CurrentUser'   { 'Current user account' }
        'LocalMachine'  { 'All users on this computer' }
        'UserPolicy'    { 'Current user (set by Group Policy)' }
        'MachinePolicy' { 'All users (set by Group Policy)' }
        default         { 'Unknown scope' }
    }
}

function Test-AdminPrivileges {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Show help if requested
if ($Help) {
    Show-Help
    exit 0
}

# Display banner
Write-Host @"
PowerShell Execution Policy Setup Tool
======================================
"@ -ForegroundColor Cyan

Write-Host ""
Write-Info "Target Configuration:"
Write-Host "  Policy Level: $PolicyLevel - $(Get-PolicyDescription $PolicyLevel)"
Write-Host "  Scope:        $Scope - $(Get-ScopeDescription $Scope)"
Write-Host ""

# Check current policies
Write-Info "Current Execution Policies:"
try {
    $policies = Get-ExecutionPolicy -List
    foreach ($policy in $policies) {
        $current = if ($policy.Scope -eq $Scope) { " (TARGET)" } else { "" }
        Write-Host "  $($policy.Scope): $($policy.ExecutionPolicy)$current"
    }
} catch {
    Write-Error "Failed to retrieve current execution policies: $_"
    exit 1
}

Write-Host ""

# Get current policy for target scope
try {
    $currentPolicy = Get-ExecutionPolicy -Scope $Scope
    Write-Info "Current policy for $Scope scope: $currentPolicy"
} catch {
    Write-Error "Failed to get current policy for scope '$Scope': $_"
    exit 1
}

# Check if change is needed
if ($currentPolicy -eq $PolicyLevel) {
    Write-Success "Execution policy is already set to '$PolicyLevel' for scope '$Scope'."
    Write-Host "No changes needed."
    exit 0
}

# Check for required privileges
if ($Scope -eq 'LocalMachine' -and -not (Test-AdminPrivileges)) {
    Write-Error "Administrator privileges required for 'LocalMachine' scope."
    Write-Host "Please run PowerShell as Administrator or use a different scope."
    exit 1
}

# Show what will be changed
Write-Warning "Proposed Change:"
Write-Host "  From: $currentPolicy"
Write-Host "  To:   $PolicyLevel"
Write-Host "  Scope: $Scope"
Write-Host ""

# Handle WhatIf mode
if ($WhatIf) {
    Write-Info "WhatIf Mode: No changes will be made."
    Write-Host "The execution policy would be changed from '$currentPolicy' to '$PolicyLevel' for scope '$Scope'."
    exit 0
}

# Confirm change unless Force is specified
if (-not $Force) {
    $confirmation = Read-Host "Do you want to apply this change? (y/N)"
    if ($confirmation -notmatch '^[Yy]') {
        Write-Host "Operation cancelled by user."
        exit 0
    }
}

# Apply the new execution policy
Write-Info "Applying new execution policy..."
try {
    Set-ExecutionPolicy -ExecutionPolicy $PolicyLevel -Scope $Scope -Force
    Write-Success "PowerShell execution policy updated successfully!"
    
    # Verify the change
    $newPolicy = Get-ExecutionPolicy -Scope $Scope
    if ($newPolicy -eq $PolicyLevel) {
        Write-Success "Verification: Policy is now set to '$newPolicy' for scope '$Scope'."
    } else {
        Write-Warning "Verification: Expected '$PolicyLevel' but got '$newPolicy'. This may be due to Group Policy overrides."
    }
    
} catch {
    Write-Error "Failed to set execution policy: $_"
    Write-Host ""
    Write-Host "Common solutions:"
    Write-Host "• Run PowerShell as Administrator (for LocalMachine scope)"
    Write-Host "• Check if Group Policy is preventing changes"
    Write-Host "• Try a different scope (Process, CurrentUser)"
    Write-Host "• Verify PowerShell version compatibility"
    exit 1
}

Write-Host ""
Write-Success "PowerShell is now configured to run scripts according to the '$PolicyLevel' policy."

# Provide usage suggestions based on policy
switch ($PolicyLevel) {
    'RemoteSigned' {
        Write-Info "With RemoteSigned policy:"
        Write-Host "• Local scripts will run without issues"
        Write-Host "• Downloaded scripts will require digital signatures"
        Write-Host "• This is the recommended setting for most users"
    }
    'Unrestricted' {
        Write-Warning "With Unrestricted policy:"
        Write-Host "• All scripts can run, but remote scripts will show warnings"
        Write-Host "• Consider using RemoteSigned for better security"
    }
    'Bypass' {
        Write-Warning "With Bypass policy:"
        Write-Host "• All scripts run without any restrictions or warnings"
        Write-Host "• This setting should be used carefully in production environments"
    }
}
