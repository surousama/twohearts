# 玩家受击表现正式化

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，把当前“玩家受击已有正式状态与时长，但仍主要是日志、HUD 与动作打断”的状态升级为正式可见的玩家受击表现链路，让受击方向、受击类型和恢复过程不再只停在调试层。

## What I already know

* `UTwoHeartsHostileAttackReceiverComponent` 已有正式 `HitReactionState`、四向方向分类、最小时长与动作中断链
* 当前仓库里能找到一些 `HitReact` 动画资源，但代码里尚未接正式玩家受击动画 / Montage
* 当前受击逻辑最主要的缺口不是“状态有没有”，而是“玩家能否稳定看到正式表现”
* 本 task 与 Guard 表现 task 相关，但职责不同：本 task 只负责玩家受击表现，不替代完整 Guard 表现

## Requirements

* 将当前 `HitReactionState` 正式接到玩家受击表现，而不是继续只靠日志和 HUD 观察
* 当前至少要补齐：
  受击方向到动画 / Montage 的正式映射；
  轻 / 重 / GuardBreak 三类受击表现区分；
  进入受击与自动恢复时的正式表现收尾
* 需要继续复用现有 `HitReactionState` 与公共动作上下文，不允许另起一套平行受击状态
* 当前允许先覆盖最小四向与最小资源集，不要求一次性做完整八向库
* 需要保留调试口径，便于确认“当前播放的是哪类受击、来自哪个方向、何时恢复”

## Acceptance Criteria

* [ ] 命中进入受击时，玩家已有正式受击表现，而不是只剩日志 / HUD
* [ ] 轻 / 重 / GuardBreak 至少已有可区分的表现配置
* [ ] Front / Back / Left / Right 至少已有稳定映射或等价最小收口
* [ ] 受击恢复时，动作上下文与表现收尾保持一致，不出现状态已结束但表现卡住
* [ ] 本轮没有越界扩散到完整角色状态机重写或完整八向动画库建设

## Out of Scope

* 完整八向 / 多武器受击大库
* 复杂霸体、受击免疫或高级连招受击规则
* 完整死亡演出
* 完整 Guard 表现

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 关键代码落点：
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
* 可复用资源线索：
  `Content/Characters/Mannequins/Anims/Rifle/HitReact/`
* 当前注意事项：
  代码已有 `HitReactionType` 与 `DirectionType`；
  本轮重点是把它们接成正式表现，不是重新定义逻辑状态

