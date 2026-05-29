# 敌方受击与生命最小闭环

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，在玩家普通攻击命中派发基础已经存在的前提下，为当前最小敌方样本建立“被玩家命中 -> 生成敌方受击结果 -> 扣减最小生命值”的正式最小闭环，使第二章基础战斗不再只有“敌打我”，而能形成双向攻防的最小正式链路。

## What I already know

* 当前 `UTwoHeartsHostileAttackReceiverComponent` 只覆盖“敌方攻击打到玩家”的接收与结果链
* 当前 `ATwoHeartsHostileAttackProbeCharacter` 是现有最小敌对样本，适合作为第一批敌方消费端
* 当前代码库中还没有与玩家普攻正式对接的敌方受击 / 生命消费入口
* 本 task 依赖 `05-28-normal-attack-hit-delivery-foundation`

## Requirements

* 为当前最小敌方样本建立正式、最小、可观察的受击与生命闭环
* 当前至少要补齐：
  敌方侧最小命中结果；
  敌方侧最小伤害结果；
  敌方侧最小生命值承载；
  `0` 血后的最小停机口径
* 需要继续沿用当前战斗元数据与公共语义层，不允许回退成散落在 Character 上的临时字段
* 需要保留清晰调试口径，至少可观察：
  命中的是哪一段普通攻击；
  扣了多少血；
  当前剩余生命值；
  `0` 血后是否仍会继续进入新受击
* 当前允许只覆盖 `HostileAttackProbe` 这一最小敌方样本，不要求一次性推广到完整敌人体系

## Acceptance Criteria

* [ ] 玩家普通攻击已能对最小敌方样本生成正式敌方命中结果
* [ ] 当前最小敌方样本已具备最小生命值承载与扣血结果
* [ ] `0` 血后已存在明确停机或忽略口径，不会无限重复落账
* [ ] 当前可从日志、HUD 或等价调试口径直接观察敌方受击 / 扣血 / 剩余生命
* [ ] 本轮没有越界扩散到完整敌方 AI、完整死亡演出或大型属性系统

## Out of Scope

* 完整敌方 AI 状态机
* 完整敌方死亡演出、掉落或尸体处理
* 多敌人共享的完整属性平台
* 联机同步

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：`05-28-normal-attack-hit-delivery-foundation`
* 当前建议优先查看：
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
* 当前允许的过渡边界：
  可先让 `HostileAttackProbe` 兼任最小敌方受击样本；
  但不允许把这轮实现写死成无法推广的局部硬编码

