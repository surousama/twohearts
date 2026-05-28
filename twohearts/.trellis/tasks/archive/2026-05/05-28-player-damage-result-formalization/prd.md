# 玩家伤害结果正式化

## Goal

作为 `05-19-chapter2-basic-combat` 之下、建立在统一攻击描述与当前玩家最小命中结果入口之上的执行子 task，把“HitConfirmed”升级成正式伤害结果与最小血量承载，让玩家第一次具备可被读取、可被改写、可被后续格挡与受击系统正式消费的伤害真相源。

## What I already know

* `05-25-player-hit-damage-minimum-loop` 已经建立了玩家侧最小结果入口，但它还不是正式伤害系统。
* 当前代码里没有看到完整的血量 / 属性 / 伤害结算体系。
* 原始框架文档要求至少支持“角色受到伤害”，这意味着当前阶段不能长期停留在“命中成立但没有正式伤害结果”的状态。
* Guard 二期中的减伤、穿透、格挡成功改写结果，也依赖一个正式伤害结果层。
* 因此，本 task 的重点是先把结果真相源立住，而不是一口气做完整 RPG 数值体系。

## Requirements

* 在现有最小命中结果入口之上，建立正式、最小、可复用的伤害结果结构。
* 当前正式伤害结果至少应包含：
  攻击来源；
  对应攻击元数据；
  本次结果类型；
  基础伤害值或最小占位值；
  最终伤害值；
  是否被 Guard 改写；
  结果时间戳。
* 需要提供最小血量承载或等价正式占位，让“命中成立”第一次能够对玩家生存结果产生正式影响。
* 若本轮暂不引入完整 `AttributeSet`，也必须给出可持续扩展的正式承载，而不是写死在临时日志里。
* Guard 成功 / 失败后续必须沿用这套伤害结果口径，不允许绕开它另起一套“格挡伤害”分支。
* 需要保留日志、HUD 或调试字段，便于观察：
  本次来袭基础伤害；
  最终伤害；
  是否被格挡改写；
  当前剩余血量或等价占位值。
* 本轮不应把任务扩大成完整属性成长、装备词条、治疗系统或完整 UI 血条。

## Acceptance Criteria

* [ ] 玩家侧已经存在正式伤害结果，而不是只有命中结果或临时打印。
* [ ] 合法来袭会生成一次可复用的正式伤害结果，并作用到最小血量承载。
* [ ] 无效 / 过期来袭不会错误地产生正式伤害结果。
* [ ] Guard 后续可以在不推翻本 task 的前提下改写伤害结果。
* [ ] 当前结果可以从日志、HUD 或调试字段中被稳定观察到。
* [ ] 本轮没有越界扩散成完整属性系统或完整 UI 系统。

## Out of Scope

* 完整属性成长系统
* 完整血条 UI、治疗系统、Buff / Debuff 数值框架
* 完整敌我双向伤害系统
* 正式受击状态与受击动画
* 完整格挡减伤 / 弹反 / 资源体系

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-28-attack-metadata-foundation`
  `05-25-player-hit-damage-minimum-loop`
* 后置依赖：
  `05-28-player-hit-reaction-minimum-implementation`
  `05-28-guard-rule-foundation-upgrade`
  `05-28-guard-outcome-settlement-and-counter-hook`
* 当前建议优先查看的代码落点：
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`
* 当前允许的过渡边界：
  允许先采用最小血量承载或可扩展占位；
  允许先不做完整 `AttributeSet`；
  但不允许继续停留在“命中成立却没有正式伤害结果”的阶段。
