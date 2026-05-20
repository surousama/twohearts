---
name: trellis-check
description: "Comprehensive quality verification: spec compliance, lint, type-check, tests, cross-layer data flow, code reuse, and consistency checks. Use when code is written and needs quality verification, before committing changes, or to catch context drift during long sessions."
---

# 双心印质量检查

## Step 1: Identify What Changed

```bash
git diff --name-only HEAD
git status
```

## Step 2: 先判定当前角色，再读相关 spec

至少读取：

```bash
cat .trellis/spec/workflow/index.md
cat .trellis/spec/roles/index.md
cat .trellis/spec/guides/index.md
```

然后按任务类型补读：

- 实现类：`combat`、`implementation`、`tasking`
- 测试类：`testing`、`tasking`
- 技术评估类：`project`、`combat`、`implementation`

## Step 3: Run Project Checks

运行当前任务实际可执行的检查命令。若本轮无法运行测试，也要明确说明原因。

## Step 4: 双心印检查清单

- [ ] 是否符合当前角色职责边界？
- [ ] 是否符合当前阶段顺序，没有提前越级做后续底座？
- [ ] 是否违背 `UE5 + GAS + C++` 主路线？
- [ ] 是否错误把核心逻辑散到蓝图或 Character 临时状态机？
- [ ] 当前目标、非目标、验收口径是否仍然一致？
- [ ] 若有新稳定经验，是否需要回写 `.trellis/spec`？

## Step 5: Report and Fix

若发现问题，优先按以下方式处理：

1. 先指出是否阻断当前阶段验收
2. 再说明问题属于：
   - 顺序问题
   - 架构边界问题
   - 实现缺陷
   - 测试口径缺失
3. 能直接修的直接修
4. 需要回上游补文档的明确指出
