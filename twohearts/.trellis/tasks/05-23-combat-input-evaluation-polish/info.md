# 战斗输入评估收口与消费边界整理

## 本轮实际完成

1. 在 `UTwoHeartsCombatActionContextComponent::EvaluateInputForAction` 中补充了显式消费路线 `ConsumptionRoute`，把“评估结果三态”和“后续由谁消费输入”拆开表达。
2. 保持对外评估结果仍为 `ExecuteNow / BufferInput / Reject`，没有改动当前首轮统一输入评估口径。
3. 将 `AtwoheartsCharacter::HandleAbilityInputPressed` 拆成：
   `HandleBufferedCombatInput`
   `TryExecuteCombatInputNow`
   `RecordAbilityInputDebugEvent`
   `RecordAbilityInputFailure`
   让角色侧主要负责分流，具体消费执行和 debug 路由分别落到更清晰的局部函数。
4. `BufferInput` 现在能在代码和调试输出里明确区分两条路径：
   `ForwardToActiveAbility`
   `ReserveForFutureBufferConsumer`
5. HUD 输入调试面板会显示 `Result / Route`，便于区分“当前已转发给 active ability”的缓冲输入，与“本轮仅保留给后续正式 preinput 消费者”的缓冲输入。

## 代码落点

1. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h`
2. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
3. `Source/twohearts/twoheartsCharacter.h`
4. `Source/twohearts/twoheartsCharacter.cpp`
5. `Source/twohearts/twoheartsDebugHUD.cpp`

## 验证

1. 已通过一次 Unreal Editor 构建验证：
   `G:\UE_5.6\Engine\Build\BatchFiles\Build.bat twoheartsEditor Win64 Development -Project=G:\twohearts\twohearts\twohearts.uproject -WaitMutex -NoHotReloadFromIDE`
2. 结果：`Succeeded`

## 当前仍保留的限制

1. `ReserveForFutureBufferConsumer` 目前只显式标出“已有缓冲评估结论，但当前无人消费”，并未在本 task 内落完整预输入缓存与后续消费机制。
2. 真实运行态的输入窗口体感、HUD 观察效果和普通攻击 / Dodge 边界，仍需交给 `05-23-combat-semantic-layer-test-pass` 在 Unreal Editor 内继续做白盒与 PIE 回归。

## Spec 判断

1. 本轮主要是在既有“Character 只保留输入桥接、公共语义层承载评估规则”的规范内做代码收口，没有形成新的稳定项目规则。
2. 结论：这次不额外更新 `.trellis/spec/`。
