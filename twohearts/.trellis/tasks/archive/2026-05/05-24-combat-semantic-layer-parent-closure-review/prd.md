# 公共战斗语义层父任务收尾判定

## Goal

作为 `05-21-combat-semantic-layer-readiness` 的主程序收口子 task，在相关实施与回归子任务已完成并归档后，复核父 task 是否已经达到可收尾状态，并输出明确的收尾结论与后续动作。

## What I already know

* 当前角色是主程序，默认以技术评估与文档收束为主，不直接写代码
* `05-21-combat-semantic-layer-readiness` 是第二章基础战斗模块下的阶段父 task
* 该父 task 已经拆出多轮子 task 推进公共战斗语义层与回归收口
* 当前工作区中已存在相关归档记录，同时父 task 仍保持 `in_progress`
* 本 task 的目标不是继续扩展语义层，而是判断父 task 是否已经具备正式收尾条件

## Requirements

* 盘点父 task 的原始目标、验收标准与已拆分子 task 的覆盖情况
* 核对子 task 是否已完成归档，以及是否存在仍未闭环的阻塞项、回归项或遗留实施缺口
* 结合当前阶段顺序，判断公共战斗语义层是否已经完成本阶段应完成的范围
* 若允许收尾，需明确父 task 应如何更新状态、沉淀结论，并指出下一阶段入口
* 若暂不允许收尾，需明确缺失项、原因与建议继续拆分方向

## Acceptance Criteria

* [ ] 明确给出父 task “可收尾 / 暂不可收尾”的主程序结论
* [ ] 已逐项对照父 task 验收标准与现有子 task/归档产物
* [ ] 已明确指出是否仍存在阻塞当前收尾的未完成项或风险
* [ ] 输出结果可直接作为父 task 后续归档或继续拆单的依据

## Out of Scope

* 新增公共战斗语义层代码实现
* 新开与本次收尾判定无关的功能设计
* 跳过现有 task / archive 证据直接主观宣布完成

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 上级父 task：`05-19-chapter2-basic-combat`
* 当前阶段顺序仍应遵循：基础闪避正式落地 -> 公共战斗语义层 -> 最小预输入
* 主要证据来源包括：父 task `prd.md`、现存 active/archived 子 task、相关回归收口 task，以及当前阶段 spec
