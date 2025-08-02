#!/bin/bash
# filepath: c:\work\libcpp\scripts\monitor-ci.sh

# 需要 GitHub CLI (gh) 工具

echo "📊 监控 CI/CD 执行状态"
echo "====================="

# 获取当前分支
BRANCH=$(git branch --show-current)
echo "监控分支: $BRANCH"

# 检查最新的工作流运行
echo ""
echo "最近的工作流运行:"
gh run list --branch $BRANCH --limit 5

# 获取最新运行的状态
LATEST_RUN_ID=$(gh run list --branch $BRANCH --limit 1 --json databaseId --jq '.[0].databaseId')

if [ -n "$LATEST_RUN_ID" ]; then
    echo ""
    echo "最新运行详情:"
    gh run view $LATEST_RUN_ID

    # 实时监控
    echo ""
    echo "🔍 实时监控中... (Ctrl+C 退出)"
    while true; do
        STATUS=$(gh run view $LATEST_RUN_ID --json status,conclusion --jq '.status')
        CONCLUSION=$(gh run view $LATEST_RUN_ID --json status,conclusion --jq '.conclusion')
        
        echo "$(date): 状态=$STATUS, 结论=$CONCLUSION"
        
        if [ "$STATUS" = "completed" ]; then
            if [ "$CONCLUSION" = "success" ]; then
                echo "✅ CI/CD 执行成功!"
            else
                echo "❌ CI/CD 执行失败: $CONCLUSION"
                echo "查看日志:"
                gh run view $LATEST_RUN_ID --log-failed
            fi
            break
        fi
        
        sleep 30
    done
else
    echo "❌ 未找到工作流运行记录"
fi