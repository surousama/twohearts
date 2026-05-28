# 格挡结果结算与反击接口预留

## Goal

作为 `05-19-chapter2-basic-combat` 之下、跟在格挡规则基础升级之后的执行子 task，补齐格挡成功后的位移结果、伤害结果、资源 / 冷却语义与反击接口预留，让 Guard 不再只有“成功 / 失败”布尔结果，而是开始具备原始框架文档所要求的两层结算能力。

## What I already know

* 当前基础 Guard 已完成首轮正式实现与白盒验收，但它仍偏向“最小规则样本”。
* 原始框架文档要求格挡成功后至少分两层结算：
  先判断位移结果；
  再判断伤害结果。
* 原始框架文档还要求为后续防守反击、资源变化、冷却语义和更完整反馈预留接口。
* 如果没有这一步，Guard 就只能停留在“命中被改写了”这一层，无法进入更完整战斗规则阶段。

## Requirements

* 在 `guard-rule-foundation-upgrade` 的正式格挡结果之上，补齐格挡成功后的最小结果结算。
* 当前至少需要覆盖：
  格挡成功后的位移结果分类；
  格挡成功后的伤害结果分类；
  最小资源 / 冷却语义；
  后续反击或后续能力读取的接口预留。
* 位移结果当前允许先采用最小分类承载，例如：
  格挡者后退；
  攻击者后退；
  双方都不后退。
* 伤害结果当前允许先采用最小分类承载，例如：
  完全挡下；
  部分承伤；
  穿透失败。
* 当前需要明确 Guard 成功后资源 / 冷却是否成立，以及它们是在成功时扣除、成功后返还，还是按其他当前阶段规则处理；但必须把语义写清，不能继续模糊。
* 需要为后续防守反击或格挡后派生能力预留一个正式读取点，不要求本轮直接实现完整反击技能。
* 需要保留最小可观察性，便于从日志、HUD 或调试字段直接看到：
  这次格挡成功后到底走了哪一种位移结果；
  走了哪一种伤害结果；
  是否开放了后续反击接口。
* 本轮不应把任务扩大成完整弹反战斗系统、完整资源条系统或完整镜头演出包。

## Acceptance Criteria

* [ ] 格挡成功后已经存在正式的位移结果与伤害结果分类，而不只是一个 `GuardSucceeded` 布尔值。
* [ ] 当前资源 / 冷却语义已经明确，并能被稳定观察到。
* [ ] 后续反击或派生能力已经有正式接口预留，而不是只能依赖零散状态判断。
* [ ] 本轮继续沿用 Guard 规则 task 与伤害结果 task 的正式口径，没有另写一套临时结算分支。
* [ ] 本轮没有越界扩散成完整弹反、完整资源条、完整镜头特效大系统。

## Out of Scope

* 完整反击技能设计与实现
* 完整弹反 / 完美格挡体系
* 完整耐力条、防御属性、装备防御成长系统
* 完整镜头震屏、后处理、特效和音效包
* 多角色联合格挡或联机同步细节

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-28-guard-rule-foundation-upgrade`
* 后置依赖：
  `05-28-preinput-second-pass-generalization`
  以及后续更完整公共规则收束 task
* 当前建议优先查看的代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.*`
  `Source/twohearts/twoheartsCharacter.*`
* 当前允许的过渡边界：
  允许先用结果分类和调试输出来表达位移 / 伤害结算；
  允许反击先只做到接口预留；
  但不允许让 Guard 长期停留在“只有成功 / 失败，没有后续结算语义”的阶段。
