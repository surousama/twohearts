# 攻击描述与命中元数据基础

## Goal

作为 `05-19-chapter2-basic-combat` 之下、位于“最小攻防闭环 + 基础 Guard 白盒验收”之后的下一直接开工子 task，建立统一的攻击描述与命中元数据基础，让后续伤害、受击、格挡不再各写一套临时判断，而是共享同一份攻击语义真相源。

## What I already know

* 当前已经存在最小敌对攻击探针、玩家侧最小受击结果入口和基础 Guard。
* 这些能力已经足够证明“最小闭环存在”，但还不足以支撑完整受击 / 伤害和完整格挡规则系统。
* 原始框架文档明确要求把“受击反应类型”与“伤害 / 机制标签类型”拆开表达，并让攻击段自身携带可格挡、可闪避规避等规则信息。
* 当前最大的结构性缺口之一，是攻击规则信息仍然过薄，容易导致后续伤害、受击、格挡各自再长出临时分支。
* 因此，本 task 的目标不是直接做伤害或受击表现，而是先补统一攻击语义底座。

## Requirements

* 建立一个正式、最小、可复用的攻击描述或命中元数据结构，供后续系统共享读取。
* 该结构至少应覆盖：
  攻击来源或实例标识；
  受击反应类型；
  伤害 / 机制标签；
  是否可格挡；
  是否可被闪避规避；
  最小方向或朝向基准；
  与当前命中窗口对应的最小时序标识。
* 本轮至少要让 `05-25-minimal-hostile-attack-probe` 能产出这份正式结构，而不是继续只靠零散字段或日志判断。
* 若当前玩家普攻还没有正式敌方受击消费侧，本轮允许只先为普攻 Ability 提供同结构的挂载点或默认描述，不强行扩成完整敌人系统。
* 后续 `player-damage-result-formalization`、`player-hit-reaction-minimum-implementation`、`guard-rule-foundation-upgrade` 必须直接读取这份结构，而不是再各自引入平行字段。
* 需要保留最小调试可读性，便于观察本次来袭到底携带了什么攻击语义。
* 本轮不应把任务扩大成完整数值系统、完整敌人框架或完整战斗 DataAsset 体系。

## Acceptance Criteria

* [x] 当前已经存在一份正式的攻击描述 / 命中元数据结构，而不是继续依赖散落条件分支。
* [x] `05-25-minimal-hostile-attack-probe` 已能稳定产出该结构。
* [x] 该结构已经明确区分“受击反应类型”与“伤害 / 机制标签类型”。
* [x] 后续伤害、受击、格挡 task 不需要再推翻重做一套新的攻击规则输入。
* [x] 本轮没有越界扩散成完整敌人系统、完整伤害公式系统或完整数据驱动平台。

## Out of Scope

* 正式伤害结算
* 玩家受击状态机或完整受击动画
* 完整敌人 AI / 敌人攻击框架重做
* 完整武器或技能攻击数据平台
* 完整格挡成功 / 失败规则结算

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 上游已完成能力：
  `05-25-minimal-hostile-attack-probe`
  `05-25-player-hit-damage-minimum-loop`
  `05-25-basic-guard-implementation`
* 后置依赖：
  `05-28-player-damage-result-formalization`
  `05-28-player-hit-reaction-minimum-implementation`
  `05-28-guard-rule-foundation-upgrade`
* 当前建议优先查看的代码落点：
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.*`
* 当前允许的过渡边界：
  允许先从最小敌对攻击样本切入；
  允许普攻先只补元数据挂载点；
  但不允许把后续系统继续建立在硬编码布尔判断之上。
