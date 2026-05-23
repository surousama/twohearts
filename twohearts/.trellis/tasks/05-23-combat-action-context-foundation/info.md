# 公共动作上下文最小底座 - 实施记录

## 当前结论

1. 已在 `Combat` 层正式新增公共动作上下文承载组件 `UTwoHeartsCombatActionContextComponent`，不再要求后续动作把公共语义先塞进 Character 私有调试字段里。
2. 已提供最小共享语义定义：
   `ETwoHeartsCombatActionType`
   `ETwoHeartsCombatActionEndReason`
   `FTwoHeartsCombatActionRegistration`
   `FTwoHeartsCombatActionContextSnapshot`
3. 已提供统一生命周期入口：
   `BeginAction`
   `TransitionToPhase`
   `MarkLogicEnded`
   `FinishAction`
4. 已提供最小公共观测口径：
   `ATwoheartsDebugHUD` 现在能直接显示公共动作上下文快照；
   组件自身会输出 `[CombatActionContext]` 结构化日志。
5. 2026-05-23 已通过一次 `twoheartsEditor Win64 Development` 构建验证。

## 本轮代码落点

1. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
   新增公共动作上下文组件、共享类型、快照结构和统一生命周期更新接口。
2. `Source/twohearts/twoheartsCharacter.*`
   在角色上正式挂载 `CombatActionContextComponent`，为后续 Ability 桥接提供稳定读取入口。
3. `Source/twohearts/twoheartsDebugHUD.cpp`
   新增 “Public Action Context” 区块，用于观察当前公共动作上下文快照。

## 当前已明确完成的边界

1. 这轮只完成“公共动作上下文底座”和公共观测口径。
2. 这轮没有提前把普通攻击完整桥接到公共动作上下文。
3. 这轮没有提前把 Dodge 生命周期或打断规则完整桥接到公共动作上下文。
4. 这轮没有引入新的 Character 级战斗状态机；正式承载仍是 `Combat` 层 `C++` 组件。

## 给后续 task 的接入约定

1. `05-23-normal-attack-semantic-bridge`
   应把普攻当前私有阶段流转接到：
   `BeginAction`
   `TransitionToPhase`
   `MarkLogicEnded`
   `FinishAction`
2. `05-23-dodge-semantic-bridge-and-interrupt-unification`
   应把 Dodge 生命周期正式接到同一组件，并在此基础上升级统一打断入口。
3. HUD 与结构化日志已准备好；后续桥接完成后，应优先用公共上下文观察值做联调，而不是继续只看各自私有布尔值。

## 当前已知限制

1. 当前组件底座已存在，但尚未由普通攻击或 Dodge 正式写入真实动作数据，因此 HUD 的公共动作上下文区域目前默认只会显示空快照。
2. 当前还没有统一打断判定接口；“是否允许被 Dodge 打断”仍留在后续 `dodge-semantic-bridge-and-interrupt-unification` task 中处理。
3. 当前还没有把输入评估接到该组件；这仍属于后续 `combat-input-evaluation-preinput-hook` 的工作范围。
