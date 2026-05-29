# 普通攻击命中派发基础

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，把当前“普攻只有动作与攻击元数据、但还没有真正把命中送到敌方”的状态升级为正式的玩家侧攻击派发基础，让普通攻击在命中窗口内能够稳定地产生可消费的攻击实例，而不是只停在 `BuildCurrentAttackMetadata()`。

## What I already know

* `UTwoHeartsGA_NormalAttackBase` 已有 `AttackMetadataTemplate` 与 `BuildCurrentAttackMetadata()`，但当前主要用于动作实例命名与后续扩展预留，还没有形成玩家打到敌人的正式派发链
* `ATwoHeartsHostileAttackProbeCharacter` 已证明“敌方攻击 -> 信号 -> receiver”的派发模式在当前项目内可工作
* 当前第二章的主要缺口之一，是普通攻击还没有接成“我打敌”的正式命中入口
* 本 task 是 `05-28-hostile-hit-and-health-minimum-loop` 的硬前置；没有稳定的玩家攻击派发，敌方侧就无从消费

## Requirements

* 为玩家普通攻击建立最小正式的命中派发基础，而不是继续只保留攻击元数据模板
* 当前至少要明确：
  普攻在什么时机视为可命中；
  同一攻击实例如何防止重复命中同一目标；
  命中时向敌方侧派发什么结构化数据
* 需要尽量复用现有攻击元数据 `FTwoHeartsAttackMetadata`，不允许重新发明一套只给玩家普攻用的临时结构
* 当前允许先只覆盖普通攻击，不要求把 Dodge、Guard 或未来技能全部纳入同一派发系统
* 需要保留清晰调试可读性，至少能说明：
  当前攻击实例名；
  命中窗口是否开启；
  命中了哪个目标；
  是否被去重忽略
* 本轮允许先采用最小命中体积、Trace 或等价方案，但必须能稳定服务后续敌方受击消费

## Acceptance Criteria

* [ ] 普通攻击在命中窗口内已经能稳定产生正式的玩家攻击派发，而不是只保留动作元数据
* [ ] 同一攻击实例对同一目标不会重复结算
* [ ] 派发数据继续沿用 `FTwoHeartsAttackMetadata` 或其正式桥接结构，而不是临时字符串 / 布尔组合
* [ ] 当前能从日志、HUD 或等价调试口径明确看到玩家普攻命中派发是否成功
* [ ] 本轮没有越界扩散到敌方完整死亡系统、完整 AI 战斗系统或完整技能命中平台

## Out of Scope

* 敌方完整生命条、死亡、霸体或受击 AI
* 玩家所有技能的一次性命中派发统一
* 完整武器碰撞系统或复杂打击检测框架
* 网络同步与联机复制

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 关键代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
* 当前判断：
  `BuildCurrentAttackMetadata()` 已经准备好了攻击描述；
  但仍缺“玩家攻击如何派发到目标”的正式桥接
* 下游直接依赖：
  `05-28-hostile-hit-and-health-minimum-loop`

