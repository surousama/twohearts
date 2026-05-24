# 首轮白盒测试报告

## 结论摘要

本轮已确认“最小预输入”不是只停留在 `ReserveForFutureBufferConsumer` 口径上，而是已经具备正式缓冲槽、正式消费者和可见调试信号。

首轮白盒结论为：

* 普攻链路已形成“评估 -> 缓冲 -> 消费”的正式实现
* Dodge 链路已接入同一套保留缓冲输入的正式消费入口
* HUD / 结构化日志已能观察 buffered input 与输入评估结果
* 但当前实现存在一个明确风险点：缓冲输入在“尝试消费”时会先被清槽，后续如果激活失败，没有统一回退机制，输入会直接丢失

## 通过项

### 1. 正式缓冲槽已建立在公共动作上下文上

证据：

* `UTwoHeartsCombatActionContextComponent::BufferInput` 会把输入类型、消费路由、原因和时间写入正式缓冲结构，而不是只写日志。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:194`
* `ConsumeBufferedInput` 会通过统一入口交给消费者读取。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:215`

结论：

* “缓冲输入存下来”这件事已经落在正式代码里，满足上游 PRD 的核心要求。

### 2. 普攻晚到输入已具备正式消费路径

证据：

* 普攻在 `Recovery/LogicEnded` 后收到晚到普攻输入时，会被评估为保留给未来消费者，而不是直接拒绝。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:157`
* 普攻段结束时会优先尝试消费晚到的 buffered normal attack。
  代码落点：`Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:315`
* 若当前段没有直接衔接成功，普攻结束后仍会走统一角色级消费者入口尝试消费保留输入。
  代码落点：`Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:346`

结论：

* 普攻主链路已经不是“只有评估，没有消费”，而是存在正式消费者。

### 3. Dodge 后续输入已接入同一套保留消费入口

证据：

* Dodge 在 `Recovery/LogicEnded` 窗口允许把后续输入评估为 buffered follow-up。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:172`
* Dodge 正常结束后，会调用与普攻相同的 `TryConsumeReservedCombatInput("DodgeEnded")`。
  代码落点：`Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:587`

结论：

* Dodge 并不是旁路实现，而是接入了同一套“保留缓冲 -> 结束时消费”的最小预输入口径。

### 4. 调试观察口径基本成立

证据：

* HUD 已显示 `buffered / input / route / buffer_reason`。
  代码落点：`Source/twohearts/twoheartsDebugHUD.cpp:162`, `:172`
* HUD 已显示 `Input Evaluation` 区域。
  代码落点：`Source/twohearts/twoheartsDebugHUD.cpp:184`
* HUD 已显示 `last_dodge_event`，有利于观察 Dodge 链路的后续消费。
  代码落点：`Source/twohearts/twoheartsDebugHUD.cpp:231`
* 角色侧会记录 `ExecuteNow / BufferInput / BufferedConsumed / BufferedConsumeFailed / Reject` 等输入事件。
  代码落点：`Source/twohearts/twoheartsCharacter.cpp:989`

结论：

* 从代码口径看，已经具备区分“立即执行 / 已缓冲 / 已消费 / 被拒绝”的基础观察能力。

## 问题项

### 1. 已复现：缓冲输入消费失败后没有统一回退，输入会直接丢失

优先级：中

触发条件：

* 某个输入已进入正式 buffered slot
* 后续消费者开始消费它
* 但真实 Ability 激活失败，或下一段能力推进失败

复现步骤：

1. 在 Dodge 或普攻的可缓冲窗口制造一个 buffered input。
2. 让消费者走到 `ConsumeBufferedInput(...)`。
3. 让后续激活失败。
   例子：
   Dodge 结束后消费的是一个当前仍会被 cooldown/tag 阻塞的输入；
   或普攻晚到输入被取出后，后续下一段推进失败。

现象：

* 缓冲槽会先被清空。
* 如果后续激活失败，没有统一恢复逻辑把输入放回缓冲槽。

预期：

* 若产品定义是“一次消费失败就应丢弃”，需要有明确规则和调试信号。
* 若产品定义是“失败后仍应保留给下一可消费时机”，则应恢复缓冲槽或改成两阶段确认消费。

实际：

* `ConsumeBufferedInput` 在统一上下文里先清空缓冲槽。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:215`
* 角色级统一消费者 `TryConsumeReservedCombatInput` 在激活失败时只记 `BufferedConsumeFailed`，不会回填缓冲输入。
  代码落点：`Source/twohearts/twoheartsCharacter.cpp:944`, `:981`
* 普攻晚到消费路径只在“取出的不是 NormalAttack”时才手动放回；如果是 NormalAttack 但后续 `AdvanceSegment` 失败，也不会恢复原缓冲。
  代码落点：`Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:386`, `:393`, `:642`, `:708`, `:717`

影响：

* 玩家在边界时机打出的后续输入可能被静默吃掉。
* 现有调试口径虽然能看到 `BufferedConsumeFailed`，但对玩家体验来说仍然是“输入丢了”。
* 这会增加后续 PIE 跑测中“偶发断连段/断后续动作”的风险。

问题说明：

* 当前实现已经有“正式消费”，但还没有“消费失败后的统一策略”。
* 这更像是最小实现阶段留下的缺口，而不是调试口径缺失。

建议修法：

* 方案 A：改成 `PeekBufferedInput + ConfirmConsume` 两阶段模型，只有真实激活成功后再清槽。
* 方案 B：保留当前接口，但在消费失败时按规则回填缓冲槽。
* 方案 C：若产品明确要求“一次失败就丢弃”，则需要把这条规则补回 PRD / spec，并强化 HUD/日志文案。

复测重点：

* Dodge 结束后接普攻
* Dodge 结束后接 Dodge
* 普攻晚到输入衔接下一段
* Ability 被 tag / cooldown 阻塞时，buffer 是否被错误清空

## 风险项

### 1. 需继续观察：Dodge 缓冲窗口对动作类型较宽，但当前统一映射仅覆盖 NormalAttack / Dodge

证据：

* 公共动作类型枚举里已存在 `Guard`。
  代码落点：`Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h:15`
* `TryGetCombatInputIDForActionType` 当前只支持 `NormalAttack` 与 `Dodge`。
  代码落点：`Source/twohearts/twoheartsCharacter.cpp:60`, `:64`, `:68`

结论：

* 当前章内主链路只测普攻和 Dodge，所以本轮不算现网问题。
* 但后续一旦 Guard 等动作也接入同一缓冲窗口，角色级统一消费者会出现“能缓冲、不能映射、最终消费失败”的扩展风险。

## 后续 PIE 建议

优先跑下面三条：

1. Dodge 结束前输入普攻，确认 `buffered -> consumed` 是否稳定出现
2. 普攻第 1 段晚到输入，确认是否稳定自动推进第 2 段
3. 人为制造一次消费失败场景，观察 HUD 中 buffered slot 是否被直接清空
