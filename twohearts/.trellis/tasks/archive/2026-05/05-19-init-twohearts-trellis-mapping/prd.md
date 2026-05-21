# 初始化双心印 Trellis 映射

## Goal

为双心印项目建立一套可长期使用的 Trellis 工作流配置，使 AI 在后续参与需求梳理、实现协作、测试分析和文档回写时，能够稳定读取正确上下文，并遵守当前项目的真实协作方式。

## What I already know

* 项目当前已有较完整的 `docs` 文档体系，包含总提示词、角色职责、项目介绍、战斗框架、开发总文档、测试文档和程序技术文档。
* 当前真实工作流是文档驱动开发：先定边界和验收，再按角色推进，再通过跑测/白盒反馈回写主文档。
* 用户希望直接进行重度配置，而不是只做最小映射。
* 当前已完成 `trellis init -u palladianli --codex -y`。
* 当前 Trellis 默认模板偏通用 Web 项目，不适合直接用于双心印。

## Assumptions

* 双心印后续会持续沿用 `UE5 + GAS + C++` 为核心技术路线。
* 当前 `docs` 目录仍然是项目已有知识的主要存放地。
* 本轮配置目标不是替代现有文档，而是让 Trellis 更好地组织和注入这些文档。

## Open Questions

* 当前无

## Requirements

* 设计适合双心印的 `.trellis/spec` 目录结构。
* 设计适合双心印的 task 使用方式与模板口径。
* 调整 workflow，使其更贴近当前项目的真实协作流程。
* 明确后续 Trellis 与现有 `docs` 的关系。
* 在 Codex 环境中保证可实际使用。
* `.trellis/spec` 逐步成为主真相源。
* 迁移过程中保留 `docs` 原文档，不删除旧内容，确保可回退。
* 在至少一个后续真实任务成功用新工作流跑通前，不停止保留旧 `docs` 内容。
* 当新工作流在下一个任务里验证跑通后，后续稳定规则优先维护在 `.trellis/spec`；`docs/` 继续维护策划类文档，开发任务和测试文档不再作为持续维护主区。
* task 粒度采用混合模式：大阶段使用父 task，具体执行使用单轮可验收子 task。
* 新 workflow 强制 AI 在开工前先判定当前角色，再决定读取哪些 spec / task 文档。

## Acceptance Criteria

* [ ] `.trellis/spec` 结构不再沿用默认前后端模板，而是适配双心印项目。
* [ ] 已明确 `docs` 与 `.trellis/spec` 的主从关系。
* [ ] 已形成一版适合双心印的 workflow 方案。
* [ ] 已形成一版适合双心印的 task 使用方案。
* [ ] 后续可以基于该配置创建真实开发任务并稳定运行。

## Out of Scope

* 立即迁移所有历史文档内容到 `.trellis/spec`
* 立即对所有既有开发阶段创建完整 task 档案
* 本轮内实现具体战斗功能代码

## Migration Strategy

* 选择方案：`.trellis/spec` 逐步成为主真相源。
* 迁移阶段：旧 `docs` 保留，不删除，不破坏原有文档体系。
* 验证阶段：需要至少一个真实任务用新工作流完整跑通。
* 验证通过后：后续稳定规则优先维护在 `.trellis/spec`；`docs/` 继续维护策划类文档，旧开发任务和测试文档仍保留用于回溯。
* task 粒度：采用父 task + 子 task 的混合模式。父 task 承载阶段目标，子 task 承载单轮可验收执行项。
* workflow 策略：AI 开工前必须先判定角色，再按角色和任务范围选择应读取的 spec / task 文档。

## Technical Notes

* 当前任务目录：`.trellis/tasks/05-19-init-twohearts-trellis-mapping/`
* 当前已安装 Trellis 版本：`0.5.17`
* 当前为 Codex inline 模式
