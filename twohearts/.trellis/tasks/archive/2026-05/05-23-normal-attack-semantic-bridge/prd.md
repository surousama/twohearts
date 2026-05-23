# 普通攻击语义桥接到公共动作上下文

## Goal

作为公共战斗语义层第二个资深程序实施子 task，把当前普通攻击内部已经稳定存在的阶段语义、逻辑结束和运行态调试，正式桥接到公共动作上下文，让普攻成为第一份真实接入公共语义层的动作样板。

## What I already know

* 本 task 只能在 `05-23-combat-action-context-foundation` 完成后开工
* 普攻当前已经在 `UTwoHeartsGA_NormalAttackBase` 内稳定维护：
  `Startup / Active / Recovery / LogicEnded`
* 普攻当前已具备：
  段序 Ability 承载；
  段内排队；
  `LogicEnded` 概念；
  Dodge 打断入口；
  调试状态输出
* 当前主程序要求本轮优先“桥接现有真实逻辑”，而不是借桥接为名重写整套普攻系统

## Requirements

* 把普通攻击当前阶段流转正式接入公共动作上下文，不允许继续只留在私有字段里做真相源
* 把普通攻击的动作开始、阶段切换、逻辑结束、完全结束与公共层接口对齐
* 尽量复用现有普攻行为与代码路径，不允许为了桥接而重写连段核心逻辑
* 当前桥接后，普通攻击仍需保持：
  第 `1/2/3` 段链路；
  段内再次输入排队；
  `LogicEnded` 语义；
  现有 Dodge 打断行为
* 当前桥接应为后续 Dodge 桥接提供真实样板，但不替 Dodge 提前实现特例逻辑
* 当前桥接后应能从公共层或统一调试口径观察到普通攻击当前动作类型与阶段

## Acceptance Criteria

* [ ] 普攻激活后会把当前动作正式登记到公共动作上下文
* [ ] 普攻 `Startup / Active / Recovery / LogicEnded` 阶段切换已同步到公共动作上下文
* [ ] 普攻结束或被打断后，公共动作上下文能正确清理或结束，不遗留脏状态
* [ ] 原有普攻 `1-2-3` 连段与段内排队行为不回归
* [ ] 后续 task 已能基于公共层读取普攻当前动作状态，而不是继续反查私有字段

## Out of Scope

* Dodge 接入公共动作上下文
* 打断规则的最终统一实现
* 输入评估与完整预输入
* 普攻资源、Montage、Notify 新一轮大改
* 受击、格挡、伤害系统接入

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 固定顺序：
  先完成 `05-23-combat-action-context-foundation`
  再做本 task
  本 task 验收通过后才允许进入 `05-23-dodge-semantic-bridge-and-interrupt-unification`
* 当前主要代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`
  `Source/twohearts/twoheartsCharacter.*`
* 推荐桥接重点：
  `ActivateAbility`
  `EnterCombatPhase`
  `TryInterruptByDodge`
  `FinishSegment`
  `EndAbility`
* 当前实现原则：
  优先把已有普攻真相源“接出去”，不是先把所有旧逻辑推翻重建
* 若发现公共动作上下文底座能力不足，应优先补底座，再继续桥接，不要在普攻侧偷偷补一份旁路状态
