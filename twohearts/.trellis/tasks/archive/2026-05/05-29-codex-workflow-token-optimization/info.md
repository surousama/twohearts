# 实现记录

## 本轮完成

1. 在 `.trellis/spec/workflow/conversation-rules.md` 中新增“低 token 工作规则”，把本项目已验证有效的低返工做法收束为正式工作流约定
2. 在 `.trellis/spec/guides/token-efficiency-thinking-guide.md` 中新增低 token 思考指南，作为后续会话的短清单入口
3. 在 `.trellis/spec/guides/index.md` 中把新指南接入索引，确保后续按 guides 层读取时可以发现
4. 收紧 `.trellis/scripts/common/workflow_phase.py` 与 `.codex/hooks/session-start.py` 的 workflow 注入范围，让 `--mode phase` / SessionStart 默认只提供 `Phase Index`，详细步骤改为按 `--step` 按需读取
5. 去重 `.codex/agents/trellis-implement.toml` 与 `trellis-check.toml` 中重复的 task context 预载说明，并补齐当前 task 的 `implement.jsonl` / `check.jsonl`，避免后续继续该任务时走空清单回退
6. 继续收紧 `.codex/hooks/session-start.py` 的默认注入内容：
   把 `<current-state>` 从完整 `get_context.py` 文本改成紧凑摘要；
   不再内联 `guides/index.md` 正文，只保留 spec index 路径按需读取；
   本地模拟后 SessionStart 注入体积约从 `11752` 字符降到 `9168` 字符

## 当前规则重点

1. 不重复全文读取 `workflow.md`
2. 搜索先收路径，再搜内容
3. 大文件优先局部读取，不默认整文件通读
4. 只有无依赖的读取命令才并行
5. PowerShell 5.1 下避免 `&&` 与高返工脚本调用写法
6. 节省 token 不能成为跳过 spec、PRD、验证与收尾的理由

## 已知边界

1. 本轮没有建立自动 token 统计系统，仍以高频浪费点收束和上下文注入减负为主
2. 本轮没有重写 `workflow.md` 主体流程，只优化了 `Phase Index` / SessionStart / agent prelude 的读取粒度
3. 本轮刻意没有把新增规则继续堆进 `AGENTS.md` 顶层提示，避免基础提示继续膨胀
4. 当前仍保留 `workflow` 的 Skill Routing / DO NOT skip / Phase 概览全文注入；若后续还要继续压缩，可再评估这部分是否值得拆成更细的按需块
