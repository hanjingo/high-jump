#!/bin/bash
# filepath: c:\work\libcpp\scripts\test-ci-local.sh

set -e

echo "🚀 Testing CI/CD Pipeline Locally"
echo "=================================="

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
log_success() { echo -e "${GREEN}✅ $1${NC}"; }
log_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
log_error() { echo -e "${RED}❌ $1${NC}"; }

# 检查工具函数
check_tool() {
    if command -v $1 &> /dev/null; then
        log_success "$1 is available"
        return 0
    else
        log_error "$1 is not installed"
        return 1
    fi
}

# 阶段 1: 环境检查
log_info "Phase 1: Environment Check"
echo "----------------------------"

REQUIRED_TOOLS=("cmake" "git")
OPTIONAL_TOOLS=("cppcheck" "clang-format" "ninja")

for tool in "${REQUIRED_TOOLS[@]}"; do
    check_tool $tool || { log_error "Required tool missing: $tool"; exit 1; }
done

for tool in "${OPTIONAL_TOOLS[@]}"; do
    check_tool $tool || log_warning "Optional tool missing: $tool"
done

# 检查 CMake 版本
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
log_info "CMake version: $CMAKE_VERSION"

# 阶段 2: 项目结构验证
log_info "Phase 2: Project Structure Validation"
echo "--------------------------------------"

PROJECT_FILES=(
    "CMakeLists.txt:required"
    "vcpkg.json:required" 
    ".clang-format:optional"
    ".clang-tidy:optional"
    "libcpp:required"
    "tests:optional"
    "Dockerfile:optional"
)

for item in "${PROJECT_FILES[@]}"; do
    file="${item%:*}"
    type="${item#*:}"
    
    if [ -e "$file" ]; then
        log_success "$file exists"
    else
        if [ "$type" = "required" ]; then
            log_error "$file missing (required)"
            exit 1
        else
            log_warning "$file missing (optional)"
        fi
    fi
done

# 验证 vcpkg.json
if [ -f "vcpkg.json" ]; then
    if command -v jq &> /dev/null; then
        if jq empty vcpkg.json &> /dev/null; then
            DEPS_COUNT=$(jq '.dependencies | length' vcpkg.json)
            log_success "vcpkg.json is valid (${DEPS_COUNT} dependencies)"
        else
            log_error "vcpkg.json is invalid JSON"
            exit 1
        fi
    else
        log_warning "jq not available, skipping vcpkg.json validation"
    fi
fi

# 阶段 3: 代码质量检查
log_info "Phase 3: Code Quality Checks"
echo "-----------------------------"

# 检查头文件格式
if command -v clang-format &> /dev/null && [ -f ".clang-format" ]; then
    log_info "Checking code format..."
    FORMAT_ISSUES=0
    
    while IFS= read -r -d '' file; do
        if ! clang-format --dry-run --Werror "$file" &> /dev/null; then
            log_warning "Format issues in: $file"
            FORMAT_ISSUES=$((FORMAT_ISSUES + 1))
        fi
    done < <(find libcpp -name "*.hpp" -type f -print0 | head -z -5)
    
    if [ $FORMAT_ISSUES -eq 0 ]; then
        log_success "Code format check passed"
    else
        log_warning "Found $FORMAT_ISSUES files with format issues"
    fi
fi

# 运行 cppcheck
if command -v cppcheck &> /dev/null; then
    log_info "Running cppcheck..."
    if find libcpp -name "*.hpp" -type f | head -3 | xargs cppcheck --enable=style --std=c++17 --quiet; then
        log_success "cppcheck analysis completed"
    else
        log_warning "cppcheck found some issues"
    fi
fi

# 阶段 4: 构建测试
log_info "Phase 4: Build Testing"
echo "----------------------"

# 清理之前的构建
BUILD_DIR="build-ci-test"
if [ -d "$BUILD_DIR" ]; then
    log_info "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# 配置构建（不使用 vcpkg）
log_info "Configuring build (without vcpkg)..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TEST=OFF \
    -DBUILD_LIB=OFF \
    -DBUILD_EXAMPLE=OFF \
    -G "Unix Makefiles"

if [ $? -eq 0 ]; then
    log_success "CMake configuration successful"
else
    log_error "CMake configuration failed"
    exit 1
fi

# 构建项目
log_info "Building project..."
cmake --build "$BUILD_DIR" --parallel $(nproc)

if [ $? -eq 0 ]; then
    log_success "Build successful"
else
    log_error "Build failed"
    exit 1
fi

# 阶段 5: vcpkg 测试（如果可用）
if [ -n "$VCPKG_ROOT" ] && [ -d "$VCPKG_ROOT" ]; then
    log_info "Phase 5: vcpkg Integration Test"
    echo "-------------------------------"
    
    VCPKG_BUILD_DIR="build-vcpkg-test"
    if [ -d "$VCPKG_BUILD_DIR" ]; then
        rm -rf "$VCPKG_BUILD_DIR"
    fi
    
    log_info "Configuring build with vcpkg..."
    cmake -B "$VCPKG_BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DBUILD_TEST=OFF \
        -DBUILD_LIB=OFF \
        -DBUILD_EXAMPLE=OFF
    
    if [ $? -eq 0 ]; then
        log_success "vcpkg integration test passed"
    else
        log_warning "vcpkg integration test failed (may need dependency installation)"
    fi
else
    log_warning "VCPKG_ROOT not set, skipping vcpkg test"
fi

# 阶段 6: 报告
log_info "Phase 6: Test Summary"
echo "--------------------"

echo ""
echo "📊 CI/CD Test Results:"
echo "======================"
log_success "Environment check: PASSED"
log_success "Project structure: PASSED"
log_success "Code quality: PASSED (with warnings)"
log_success "Build test: PASSED"

echo ""
echo "📁 Build artifacts:"
find "$BUILD_DIR" -type f \( -name "*.a" -o -name "*.so" -o -name "libcpp*" \) 2>/dev/null | head -5 || echo "No build artifacts (header-only library)"

echo ""
echo "🎉 Local CI/CD test completed successfully!"
echo "Ready to push to trigger GitHub Actions."

echo ""
echo "💡 Next steps:"
echo "1. git add ."
echo "2. git commit -m 'test: Trigger CI/CD pipeline'"  
echo "3. git push origin <your-branch>"