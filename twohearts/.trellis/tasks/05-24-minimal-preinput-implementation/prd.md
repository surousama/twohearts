# 最小预输入正式实施拆分

## Goal

作为第二章基础战斗模块下、公共战斗语义层之后的下一直接开工子 task，从主程序视角把“最小预输入”拆成可直接派给资深程序实施的正式文档，明确当前目标、边界、模块落点与验收口径。

## What I already know

* `05-21-combat-semantic-layer-readiness` 已归档完成
* 当前已经存在统一输入评估口径 `ExecuteNow / BufferInput / Reject`
* 当前已经存在最小预输入接入口，但还没有正式缓冲队列与消费机制
* 当前第二章主线顺序已经推进到：
  最小预输入
  -> 受击与伤害
  -> 格挡
* 当前角色为主程序，目标是产出可直接实施的文档，而不是本轮直接写业务代码

## Requirements

* 明确“最小预输入”本轮只做什么，不做什么
* 明确当前预输入必须建立在哪些现有正式底座之上
* 拆出首轮实施模块、推荐顺序与关键代码落点
* 写清验收标准，确保资深程序能按该文档直接开工并单轮验收
* 输出必须与当前项目主顺序对齐，不提前掺入格挡、受击、完整技能系统需求

## Acceptance Criteria

* [ ] 已形成可直接派给资深程序的最小预输入实施 `prd.md`
* [ ] 已明确本轮范围、非范围、模块拆分与验收标准
* [ ] 已写清与公共战斗语义层、后续受击/格挡阶段的关系

## Out of Scope

* 本轮直接实现最小预输入代码
* 直接把格挡、受击、技能输入统一一并做掉
* 为状态核对再新建 review task

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 当前上游已完成阶段：
  `05-21-combat-semantic-layer-readiness`
* 主要参考代码落点预计包括：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
* 对应旧文档：`../docs/a双心印开发进度简介.md`
