# 普通攻击语义桥接到公共动作上下文 - 实施记录

## 当前结论

1. 已在 `UTwoHeartsGA_NormalAttackBase` 内正式接入 `UTwoHeartsCombatActionContextComponent`
2. 普攻进入 `Startup / Active / Recovery / LogicEnded` 时，会同步更新公共动作上下文
3. 普攻结束时会根据真实结束方式写回统一结束原因：
   `Completed / Cancelled / Interrupted`
4. 普攻公共动作上下文会暴露：
   `ActionType=NormalAttack`
   `AbilityTag=Ability.NormalAttack.Segment1/2/3`
   `ActionStateTag=State.Action.NormalAttack`
   `ActionInstanceName=NormalAttack.Segment1/2/3`

## 本轮代码落点

1. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   新增公共动作上下文桥接所需的辅助函数与状态位
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
   在普攻 phase 进入与 Ability 结束路径中，正式驱动：
   `BeginAction`
   `TransitionToPhase`
   `MarkLogicEnded`
   `FinishAction`

## 当前明确边界

1. 本轮只桥接普通攻击，不提前实现 Dodge 生命周期写入公共动作上下文
2. 本轮不改 Dodge 打断判定入口；仍由后续 `05-23-dodge-semantic-bridge-and-interrupt-unification` 继续统一
3. 本轮不实现输入评估与预输入；后续由 `05-23-combat-input-evaluation-preinput-hook` 承接

## 给后续 task 的接口口径

1. 后续 Dodge task 可以直接读取公共动作上下文判断当前是否存在活跃动作、动作类型、阶段和逻辑结束状态
2. 后续统一打断入口应优先消费公共动作上下文，而不是继续反查普攻私有调试字段
3. HUD 与结构化日志现在应能直接观察到普攻真实写入的公共动作上下文快照
