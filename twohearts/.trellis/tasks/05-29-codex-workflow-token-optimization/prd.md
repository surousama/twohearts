# Codex 工作流 token 消耗优化

## Goal

作为 `00-bootstrap-guidelines` 之下的新执行子 task，把本项目当前 Trellis + Codex 的高频无效 token 消耗点整理成正式、可复用的工作流约定，并把这些约定落到 `.trellis/spec/` 的合适位置，让后续会话在不牺牲质量门槛的前提下，减少重复读大文档、过宽搜索、整文件通读和无效命令重试。

## What I already know

* 刚完成的 `05-28-normal-attack-hit-delivery-foundation` 里，主要 token 消耗并不在代码编写，而在流程上下文加载、过宽 `rg` 搜索、整文件读取和若干次环境 / 命令重试
* 当前项目已经有较完整的 `.trellis/spec/workflow/` 与 `.trellis/spec/guides/`，适合把这类经验沉淀为长期规则，而不是只停留在一次性口头建议
* 当前 `AGENTS.md` 与每轮注入内容已经较长，不适合继续把大量“省 token 经验”直接堆到顶层全局提示里，否则可能反向增加每轮基础开销
* 当前环境里至少有两个稳定坑位值得固化：
  Windows PowerShell 5.1 会拦截脚本执行；
  PowerShell 5.1 不支持 `&&`

## Requirements

* 为本项目补一轮正式的 Codex / Trellis 低 token 工作流优化，而不是只给出临时建议
* 当前至少要明确：
  哪些 token 消耗属于高频、可避免的浪费；
  后续会话应如何缩小读取范围、搜索范围与验证范围；
  在当前 Windows PowerShell 环境下，哪些命令写法应默认避免
* 优化结果必须优先落在 `.trellis/spec/` 或当前项目已有工作流承载位置中，方便未来 session 自动复用
* 需要避免把“节省 token”优化成“偷读规范”或“跳过质量门槛”；本轮优化必须保持 Trellis 基本流程与质量检查要求
* 需要尽量选择低成本、高收益的改动，不在本轮引入大规模 hook、脚本重构或复杂统计系统

## Acceptance Criteria

* [ ] 已新增或更新正式 spec / guide，明确本项目里常见的高 token 浪费点与推荐替代做法
* [ ] 后续会话已经能从规范层直接得到至少以下约定：
  先收路径再搜内容；
  少读整文件，多读命中片段；
  避免重复读取 `workflow.md` 全文；
  PowerShell 5.1 下避免 `&&` 与直接执行受限脚本
* [ ] 本轮优化没有把关键流程要求削成“为了省 token 可以跳过”
* [ ] 本轮改动不把大量新规则塞进全局顶层提示，避免基础提示继续膨胀
* [ ] 当前改动能够直接服务后续 Codex session，而不是只停留在一次性说明

## Out of Scope

* 建立完整 token 统计平台或自动计量系统
* 重写 Trellis runtime hook、注入器或 `get_context.py` 主逻辑
* 为所有平台同时做大规模模板统一改造
* 对正在进行中的战斗子 task 做实现层修改

## Technical Notes

* 父 task：`00-bootstrap-guidelines`
* 当前优先查看：
  `.trellis/spec/workflow/conversation-rules.md`
  `.trellis/spec/guides/index.md`
  `.trellis/spec/implementation/document-maintenance.md`
  `.agents/skills/trellis-start/SKILL.md`
* 当前建议优先把规则写进：
  `workflow` 层的对话 / 阅读范围约定；
  `guides` 层的“低 token 工作方式”清单
* 当前判断：
  这轮更像“把经验收束成规则”，而不是“增加更多全局提示”
