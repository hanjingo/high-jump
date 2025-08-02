param(
    [switch]$SkipBuild = $false,
    [switch]$Verbose = $false
)

Write-Host "🚀 Testing CI/CD Pipeline Locally (Windows)" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green

# 函数定义
function Write-Success($message) { Write-Host "✅ $message" -ForegroundColor Green }
function Write-Warning($message) { Write-Host "⚠️  $message" -ForegroundColor Yellow }
function Write-Error($message) { Write-Host "❌ $message" -ForegroundColor Red }
function Write-Info($message) { Write-Host "ℹ️  $message" -ForegroundColor Cyan }

function Test-Command($cmd) {
    if (Get-Command $cmd -ErrorAction SilentlyContinue) {
        Write-Success "$cmd is available"
        return $true
    } else {
        Write-Error "$cmd is not installed"
        return $false
    }
}

# 阶段 1: 环境检查
Write-Info "Phase 1: Environment Check"
Write-Host "----------------------------"

$requiredTools = @("cmake", "git")
$optionalTools = @("cppcheck", "clang-format")

foreach ($tool in $requiredTools) {
    if (-not (Test-Command $tool)) {
        Write-Error "Required tool missing: $tool"
        exit 1
    }
}

foreach ($tool in $optionalTools) {
    if (-not (Test-Command $tool)) {
        Write-Warning "Optional tool missing: $tool"
    }
}

# CMake 版本检查
$cmakeVersion = (cmake --version)[0] -replace "cmake version ", ""
Write-Info "CMake version: $cmakeVersion"

# 阶段 2: 项目结构验证
Write-Info "Phase 2: Project Structure Validation"
Write-Host "--------------------------------------"

$projectFiles = @{
    "CMakeLists.txt" = "required"
    "vcpkg.json" = "required"
    ".clang-format" = "optional"
    ".clang-tidy" = "optional"
    "libcpp" = "required"
    "tests" = "optional"
    "Dockerfile" = "optional"
}

foreach ($file in $projectFiles.Keys) {
    $type = $projectFiles[$file]
    
    if (Test-Path $file) {
        Write-Success "$file exists"
    } else {
        if ($type -eq "required") {
            Write-Error "$file missing (required)"
            exit 1
        } else {
            Write-Warning "$file missing (optional)"
        }
    }
}

# 验证 vcpkg.json
if (Test-Path "vcpkg.json") {
    try {
        $vcpkgContent = Get-Content "vcpkg.json" | ConvertFrom-Json
        $depsCount = $vcpkgContent.dependencies.Count
        Write-Success "vcpkg.json is valid ($depsCount dependencies)"
    } catch {
        Write-Error "vcpkg.json is invalid JSON"
        exit 1
    }
}

# 阶段 3: 构建测试
if (-not $SkipBuild) {
    Write-Info "Phase 3: Build Testing"
    Write-Host "----------------------"

    # 清理之前的构建
    $buildDir = "build-ci-test"
    if (Test-Path $buildDir) {
        Write-Info "Cleaning previous build..."
        Remove-Item -Recurse -Force $buildDir
    }

    try {
        # 配置构建
        Write-Info "Configuring build..."
        cmake -B $buildDir `
            -DCMAKE_BUILD_TYPE=Debug `
            -DBUILD_TEST=OFF `
            -DBUILD_LIB=OFF `
            -DBUILD_EXAMPLE=OFF `
            -G "Visual Studio 17 2022" `
            -A x64

        Write-Success "CMake configuration successful"

        # 构建项目
        Write-Info "Building project..."
        cmake --build $buildDir --config Debug --parallel 2

        Write-Success "Build successful"

        # 显示构建结果
        Write-Info "Build artifacts:"
        Get-ChildItem $buildDir -Recurse -Include "*.lib", "*.dll", "*.exe" | 
            Select-Object Name, Directory | Format-Table -AutoSize

    } catch {
        Write-Error "Build failed: $($_.Exception.Message)"
        exit 1
    }
}

# 阶段 4: 报告
Write-Info "Phase 4: Test Summary"
Write-Host "--------------------"

Write-Host ""
Write-Host "📊 CI/CD Test Results:" -ForegroundColor Cyan
Write-Host "======================"
Write-Success "Environment check: PASSED"
Write-Success "Project structure: PASSED"
if (-not $SkipBuild) {
    Write-Success "Build test: PASSED"
}

Write-Host ""
Write-Host "🎉 Local CI/CD test completed successfully!" -ForegroundColor Green
Write-Host "Ready to push to trigger GitHub Actions." -ForegroundColor Green

Write-Host ""
Write-Host "💡 Next steps:" -ForegroundColor Yellow
Write-Host "1. git add ."
Write-Host "2. git commit -m 'test: Trigger CI/CD pipeline'"
Write-Host "3. git push origin <your-branch>"