# 不可格挡与 GuardBreak 规则实现

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，把当前 Guard 规则从“可格挡攻击的时间 / 距离 / 朝向判定”继续扩展到“不可格挡与 GuardBreak 分支”的正式规则层，使后续完整 Guard 表现有稳定、明确、可复用的结果输入。

## What I already know

* 当前 Guard 已具备：
  可格挡开关；
  距离 / 高度 / 朝向判定；
  成功后的 `GuardOutcome`
* 当前明显还缺：
  不可格挡攻击的正式分支；
  GuardBreak 的正式分支；
  这两类分支对应的正式命中 / 伤害 / 受击结果
* 完整 Guard 表现 task 依赖本 task 的结果分支稳定下来，否则表现输入会持续变动

## Requirements

* 在现有 Guard 规则基础上，正式补齐不可格挡与 GuardBreak 分支
* 当前至少要明确：
  什么攻击属于不可格挡；
  什么攻击会触发 GuardBreak；
  这两类分支分别落成什么 `HitResult / DamageResult / HitReactionType`
* 需要继续沿用 `FTwoHeartsAttackMetadata` 作为攻击规则输入，不允许把不可格挡 / GuardBreak 只写成 Guard Ability 内部私有判断
* 需要保留最小调试可读性，至少能观察：
  本次攻击为何不可格挡；
  本次攻击是否触发 GuardBreak；
  玩家最终落成的结果类型
* 当前允许先覆盖 `HostileAttackProbe` 样本与最小攻击元数据配置，不要求立即推广到完整敌人库

## Acceptance Criteria

* [ ] 攻击元数据已具备不可格挡 / GuardBreak 的正式规则承载，或存在等价正式桥接字段
* [ ] Guard 面对不可格挡攻击时不再混入普通失败分支，而有独立正式结果
* [ ] GuardBreak 已能正式驱动玩家结果与受击类型，而不是只剩日志说明
* [ ] 当前可从日志、HUD 或等价调试口径明确区分：
  Guard 成功；
  普通失败；
  不可格挡；
  GuardBreak
* [ ] 本轮没有越界扩散到完整表现包或完整资源制作

## Out of Scope

* 完整 Guard 表现包
* 完整反击技能设计
* 耐力条 / 完整防御资源系统
* 全敌人类型一次性接入

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 与下游关系：
  本 task 是 `05-28-full-guard-presentation-implementation` 的硬前置之一
* 关键代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`

