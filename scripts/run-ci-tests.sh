#!/bin/bash
# filepath: c:\work\libcpp\scripts\run-ci-tests.sh

echo "🚀 执行 CI/CD 测试计划"
echo "===================="

# 步骤 1: 本地验证
echo "步骤 1: 本地验证"
echo "==============="
./scripts/test-ci-local.sh

if [ $? -ne 0 ]; then
    echo "❌ 本地测试失败，请修复后再继续"
    exit 1
fi

# 步骤 2: 创建测试分支
echo "步骤 2: 创建测试分支"
echo "=================="
BRANCH_NAME="test-ci-$(date +%Y%m%d-%H%M%S)"
git checkout -b $BRANCH_NAME

# 添加测试工作流
cp .github/workflows/ci.yml .github/workflows/ci-test.yml

# 步骤 3: 提交并推送
echo "步骤 3: 提交并推送"
echo "=================="
git add .
git commit -m "test: Add CI test workflow for validation

- Add simplified CI test workflow
- Test basic build functionality  
- Validate project structure
- Check code quality tools"

git push origin $BRANCH_NAME

echo ""
echo "✅ 测试分支已推送: $BRANCH_NAME"
echo "🔗 查看 GitHub Actions: https://github.com/$(git config remote.origin.url | sed 's/.*github.com[:/]\(.*\)\.git/\1/')/actions"

# 步骤 4: 监控结果
echo "步骤 4: 监控 CI 结果"
echo "=================="
echo "请在 GitHub 上检查以下项目："
echo "1. ✅ 工作流是否触发"
echo "2. ✅ 各个作业是否成功执行"
echo "3. ✅ 代码质量检查是否通过"  
echo "4. ✅ 构建是否成功"
echo "5. ✅ 依赖安装是否正常"

echo ""
echo "如果一切正常，可以创建 Pull Request 进行最终测试"