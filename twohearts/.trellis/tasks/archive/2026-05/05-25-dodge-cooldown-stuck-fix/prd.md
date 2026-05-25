# Dodge冷却卡死修复

## Goal

作为第二章基础战斗模块下的一个局部修复子 task，定位并修复 Dodge 在首轮成功后后续输入长期无反馈的问题，确保 `Cooldown.Dodge` 能稳定回收，不再出现“第一次能闪，之后一直不能闪”的卡死表现。

## What I already know

* 当前问题表现稳定可复现：
  第一次 Dodge 可以正常触发；
  之后再次按 Dodge 没有动作反馈；
  HUD 中 `cooldown_ready` 持续为 `NO`。
* 当前失败截图中可见：
  `Public Action Context` 已经到 `end=Completed` / `reason=DodgeEnded`；
  输入评估仍给到 `ExecuteNow / ActivateMatchingAbility`；
  但 Ability 激活最终失败；
  说明更像是正式激活阻断口径没有解除，而不是输入没有进来。
* 当前 `UTwoHeartsGA_Dodge` 的冷却是：
  在 `ApplyDodgeCooldown` 中给 ASC 加 `TAG_TwoHearts_Cooldown_Dodge`；
  再依赖 Ability 实例上的 `DodgeCooldownTimerHandle` 回调 `HandleDodgeCooldownFinished` 去清理。
* `UTwoHeartsGameplayAbility` 当前实例化策略是 `InstancedPerActor`，这意味着 Dodge 冷却清理口径与 Ability 生命周期绑定较紧，存在“动作已结束但冷却清理回调未可靠返回”的风险。
* 上游历史中已有“Dodge 冷却永久卡死问题本轮暂未复现”的记录，说明这不是全新类型的问题，而是已有隐藏问题重新暴露。
* 本轮不应顺手重做 Dodge 表现衔接、后摇状态机或预输入链路，范围只收敛在“冷却为什么不回来，以及怎样让它稳定回来”。

## Requirements

* 从主程序视角确认并修复：当前 Dodge 首次成功后后续长期不可再次激活，根因是冷却回收口径不稳定，而不是输入路由、方向解析或表现资产问题。
* 找到 `Cooldown.Dodge` 的正式真相源与回收责任归属，使其不再依赖一个容易失活或不再可靠响应的临时回调路径。
* 修复后必须保证：
  首次 Dodge 成功后，冷却到时能稳定清理；
  第二次及后续 Dodge 在冷却结束后可以再次成功触发。
* 修复过程中不破坏现有：
  Dodge 方向解析；
  Root Motion；
  无敌帧；
  公共动作上下文完成链路；
  普攻被 Dodge 打断的现有语义。
* 若最终方案需要把冷却清理责任迁到角色、ASC、公共组件或别的更稳定承载点，必须保持真相源唯一，不引入双写或“双重清理”竞态。
* 需要保留可读调试口径，让开发者能区分：
  冷却仍在；
  冷却已结束但输入仍被别的 Tag/状态阻断；
  Ability 本身未通过激活条件。

## Acceptance Criteria

* [ ] 首次 Dodge 成功后，HUD / 日志中的冷却状态会在预期时间后恢复为 ready。
* [ ] 冷却结束后再次按 Dodge，Ability 能再次正常激活，不再出现“第一次成功，之后永久失败”。
* [ ] 若第二次 Dodge 失败，调试口径能够明确指出是冷却未清、阻断 Tag 未释放、资源缺失，还是别的激活失败原因。
* [ ] 本轮没有破坏现有 Dodge 的方向、位移、无敌帧、动作完成和打断链路。
* [ ] 本轮不引入新的 Character 级临时状态机，不把冷却规则偷偷散到蓝图分支里。

## Out of Scope

* 修复 Dodge 后移动衔接表现
* 重做 Dodge 的后摇承载方案
* 重做最小预输入、状态机总结构或公共战斗语义层
* 受击、格挡、联机同步或其他战斗子系统改造

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 当前任务性质：
  最小预输入跑通后的一个基础稳定性修复，不改变第二章主顺序。
* 建议优先代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.cpp`
  `Source/twohearts/twoheartsCharacter.h`
  `Source/twohearts/twoheartsCharacter.cpp`
* 建议优先验证口径：
  1. 首次 Dodge 触发后是否稳定看到冷却开始
  2. 冷却结束时 `Cooldown.Dodge` 是否真的从 ASC 上移除
  3. 第二次 Dodge 失败时 `CanActivateAbility` 的失败标签具体是什么
* 对应已归档参考：
  `archive/2026-05/05-21-dodge-resource-local-acceptance`
  `archive/2026-05/05-23-dodge-semantic-bridge-and-interrupt-unification`
