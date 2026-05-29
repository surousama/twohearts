# 实现记录

## 本轮完成

1. 在 `.trellis/spec/workflow/conversation-rules.md` 中新增“低 token 工作规则”，把本项目已验证有效的低返工做法收束为正式工作流约定
2. 在 `.trellis/spec/guides/token-efficiency-thinking-guide.md` 中新增低 token 思考指南，作为后续会话的短清单入口
3. 在 `.trellis/spec/guides/index.md` 中把新指南接入索引，确保后续按 guides 层读取时可以发现

## 当前规则重点

1. 不重复全文读取 `workflow.md`
2. 搜索先收路径，再搜内容
3. 大文件优先局部读取，不默认整文件通读
4. 只有无依赖的读取命令才并行
5. PowerShell 5.1 下避免 `&&` 与高返工脚本调用写法
6. 节省 token 不能成为跳过 spec、PRD、验证与收尾的理由

## 已知边界

1. 本轮没有改 `get_context.py`、hook 注入器或 `workflow.md` 主体运行时逻辑
2. 本轮没有建立自动 token 统计系统，只收束人工工作方式
3. 本轮刻意没有把新增规则继续堆进 `AGENTS.md` 顶层提示，避免基础提示继续膨胀
