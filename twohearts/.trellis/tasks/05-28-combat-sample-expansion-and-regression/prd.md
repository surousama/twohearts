# 攻防样本扩展与回归验收

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，在当前多轮逻辑与表现子 task 完成后，扩展攻击样本、Guard 样本和玩家 / 敌方攻防回归场景，形成后续能直接复用的最小回归资产与白盒验收口径，避免第二章战斗继续只依赖单一 `HostileAttackProbe` 样本做判断。

## What I already know

* 当前很多闭环都建立在 `HostileAttackProbe` 的最小样本之上
* 随着不可格挡、GuardBreak、完整 Guard 表现、玩家输出到敌方等能力加入，单一样本很快不够覆盖
* 当前 repo 已有多个 probe 蓝图预设命名，但覆盖维度仍不够完整

## Requirements

* 扩展可直接用于 PIE / 白盒回归的攻防样本，而不是继续只靠单一慢速单段样本
* 当前至少要覆盖：
  普通可格挡攻击；
  不可格挡攻击；
  GuardBreak 攻击；
  PartialDamageTaken 类 Guard 结果；
  玩家普通攻击命中敌方样本
* 需要同步补齐回归口径，至少能说明：
  这次样本触发了什么攻击规则；
  结果是否符合预期；
  表现是否有明显串线
* 当前允许继续以白盒为主，不要求一次性做自动化测试平台

## Acceptance Criteria

* [ ] 当前至少存在多种可直接运行的攻防样本，覆盖 Guard 成功 / 失败 / 不可格挡 / GuardBreak / 玩家攻击命中敌方
* [ ] 已存在一份可复用的回归验收口径或记录文档，后续不必每次重新发明测试步骤
* [ ] 当前可从日志、HUD、表现结果三层确认样本是否符合预期
* [ ] 本轮没有越界扩散到完整自动化测试框架或全敌人模板平台

## Out of Scope

* 完整自动化战斗测试框架
* 全敌人家族一次性覆盖
* 完整数据驱动策划工具链

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 建议放在以下 task 之后：
  `05-28-hostile-hit-and-health-minimum-loop`
  `05-28-player-hitreaction-presentation-formalization`
  `05-28-guard-unguardable-and-guardbreak-rules`
  `05-28-full-guard-presentation-implementation`
* 关键落点：
  `Content/Characters/HostileAttackProbe/`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`

