# 战斗属性真相源收口

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，把当前分散在玩家 / 敌方接收组件中的最小生命值和后续 Guard 资源语义收束为更稳定的战斗状态真相源，避免在双向攻防都成立后继续沿用“谁接收攻击谁自己临时记血”的过渡结构。

## What I already know

* 当前玩家生命值直接挂在 `UTwoHeartsHostileAttackReceiverComponent`
* 一旦敌方侧也建立最小生命值承载，就很容易出现玩家侧一套、敌方侧一套、后续 Guard 资源又再来一套的分散状态
* 当前阶段还不一定要一次性上完整大型属性系统，但必须尽快收口“谁是生命 / 防御 / 资源真相源”

## Requirements

* 为第二章当前基础战斗建立更稳定的战斗状态真相源，而不是继续把关键数值分散在多个接收组件里
* 当前至少要明确：
  生命值真相源放在哪里；
  玩家与敌方最小生命值如何共用口径；
  Guard 资源 / 冷却后续该从哪里读取
* 当前允许先采用最小正式层，而不是一步到位做完整大型 `AttributeSet` 平台
* 需要尽量减少对已完成的攻防闭环与表现 task 的回滚影响
* 需要保留调试可读性，至少能持续观察生命值 / 结果落账 / 关键资源语义

## Acceptance Criteria

* [ ] 生命值真相源已不再只散落在单一接收组件内部
* [ ] 玩家与敌方最小生命值已有统一或等价一致的正式承载口径
* [ ] Guard 冷却 / 未来资源语义的读取位置已更稳定，不再只是一轮 task 的局部字段
* [ ] 当前攻防闭环与调试口径未被回退
* [ ] 本轮没有越界扩散到大型通用 RPG 属性系统

## Out of Scope

* 完整 RPG 属性系统
* 完整装备 / 数值成长体系
* 联机同步属性复制
* 完整 UI 属性框架

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 建议优先查看：
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  `Source/twohearts/twoheartsCharacter.*`
* 当前判断：
  这是中期稳定性 task；
  不应早于双向攻防闭环和关键表现 task 开始，但也不宜无限后拖

