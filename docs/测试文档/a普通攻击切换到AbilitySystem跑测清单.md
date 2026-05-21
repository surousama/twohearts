# 普通攻击切换到 AbilitySystem 跑测清单

## 1. 文档用途

1. 本文档用于《双心印》当前阶段“普通攻击完全切换到 AbilitySystem”后的手动跑测与联调验证。
2. 本文档面向策划与资深测试，重点确认普通攻击是否已经真正切换到 `AbilitySystem` 正式承载。
3. 当前验收口径为单角色本地验证。

## 2. 测试目标

1. 确认当前普通攻击已正式切换到 `AbilitySystem` 承载。
2. 确认 1-2-3 普攻连段在当前实现下能够稳定闭环。
3. 确认运行时不再依赖旧 `Character` 普攻状态机作为正式路径。
4. 确认当前实现具备继续进入“阶段标记与基础打断”阶段的基础稳定性。

## 3. 测试前提

1. 工程可正常编译并启动编辑器。
2. 当前测试角色为 `AtwoheartsCharacter` 体系。
3. 当前角色蓝图或角色实例已正确配置：
   `NormalAttackAction`
   `NormalAttackMontage`
   `NormalAttackSectionNames`
4. `NormalAttackSectionNames` 当前预期为：
   `Attack_1`
   `Attack_2`
   `Attack_3`
5. 当前 Montage 内实际存在上述 3 个 Section。
6. 当前项目已启用 `GameplayAbilities` 插件，并已接入：
   `GameplayAbilities`
   `GameplayTags`
   `GameplayTasks`

## 4. 阶段A：GAS 底座验证

1. 进入游戏后，角色应能正常生成，不因 `AbilitySystem` 初始化异常导致角色不可操作。
2. 按下普攻输入后，不应出现“完全无反应且无 Ability 链路表现”的情况。
3. 如查看日志，应能确认 `AbilitySystem` 已初始化 `ActorInfo`。
4. 如查看日志，应能确认默认战斗 Ability 已成功授予。
5. 如查看日志，应能确认普攻输入已进入 `HandleAbilityInputPressed` 链路。
6. 当前默认普攻 Ability 应为正式三段 Ability，而不是旧测试 Ability。

## 5. 阶段B：普通攻击功能行为验证

1. 点击一次普通攻击，只播放第 1 段。
2. 在第 1 段播放期间再次点击，能够稳定进入第 2 段。
3. 在第 2 段播放期间再次点击，能够稳定进入第 3 段。
4. 第 1 段结束前未继续点击时，角色正确回待机。
5. 第 2 段结束前未继续点击时，角色正确回待机。
6. 第 3 段结束后，角色正确结束整套普通攻击并回待机。
7. 连段完整结束后再次点击，能够重新从第 1 段开始。
8. 多次快速点击时，当前阶段只缓存一个“下一段请求”，不应出现多段排队。
9. 普攻执行过程中，不应出现段号错乱、跳段、重复段或卡死在攻击状态。
10. 普攻执行结束后，角色应恢复正常待机与移动能力。

## 6. 阶段C：Ability 承载正确性验证

1. 普攻输入全程应由 `AbilitySystem` 驱动，不应回落到旧 `Character` 普攻路径。
2. 第 1 段、第 2 段、第 3 段的进入与结束，应由 Gameplay Ability 主导。
3. 当前段结束后的下一段衔接，应由 Ability 内逻辑完成，而不是 `Character` 帮忙推进。
4. 当前普攻时序应基于 `UAbilityTask_PlayMontageAndWait` 的完成或打断回调，而不是旧 `Timer`。
5. 运行中不应再依赖以下旧逻辑：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`

## 7. 阶段D：调试面板与日志校验

1. 调试面板中的 `attacking` 应与是否处于普攻执行中一致。
2. 调试面板中的 `segment` 应与当前实际播放段数一致。
3. 调试面板中的 `queued_next` 应与是否已缓存下一段请求一致。
4. 调试面板中的 `latest_section` 应与当前 Montage Section 一致。
5. `Recent Events` 应能看到合理的事件顺序，例如：
   `PlaySegment`
   `QueueNextSegment`
   `SegmentFinished`
   `AdvanceSegment`
6. 如出现失败，`Last Failure` 应能提供明确失败原因，而不是无信息失败。

## 8. 异常与边界情况

1. 第 1 段期间连续快速点击，不应直接跳到第 3 段。
2. 第 2 段期间连续快速点击，不应出现第 3 段重复触发。
3. 第 3 段期间继续点击，不应再缓存不存在的下一段。
4. 普攻结束瞬间再次点击，应能正常重新起第 1 段，不应无响应。
5. 如果 `Montage`、`Section` 或 `AnimInstance` 配置错误，应能从日志或调试面板快速定位问题。
6. 即使本次普攻失败，也不应导致角色后续输入永久失效。

## 9. 重点风险关注

1. 第 1 段能播，但第 2 段接不上。
2. 面板显示 `queued_next=true`，但没有实际进入下一段。
3. Ability 激活失败，但表面现象像是动画没播。
4. 表面已切到 GAS，实际仍依赖旧 `Character` 状态机。
5. 快速点击时出现多段排队，超出当前“只缓存一次下一段请求”的设计口径。

## 10. 本轮测试结论记录建议

1. 是否通过 GAS 底座验证。
2. 是否通过 1-2-3 普攻完整闭环验证。
3. 是否确认已完全切离旧 `Character` 普攻正式路径。
4. 是否存在高频复现问题、偶现问题或资源配置问题。
5. 是否具备进入下一阶段“阶段标记与基础打断”的测试前提。

## 11. 白盒评审发现的问题

### 11.1 风险1：首击可能错误进入第 2 段或第 3 段

1. 风险等级：
   高风险
2. 问题描述：
   当前第 2 段和第 3 段 Ability 没有看到独立的“只能由上一段推进进入”的激活前置约束。
   当前首击能否一定从第 1 段开始，较依赖 `DefaultCombatAbilities` 的授予顺序，以及遍历时第 1 个成功激活的 Ability，而不是代码里显式保证“首击只能进入第 1 段”。
3. 可能影响：
   一旦默认授予顺序被改乱、配置遗漏第 1 段，或者第 1 段因资源问题激活失败，首击存在误进第 2 段或第 3 段的可能。
   这会直接破坏当前 1-2-3 普攻链路的正确性。
4. 代码定位：
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.cpp`
5. 建议验证方向：
   故意改乱 `DefaultCombatAbilities` 顺序。
   故意让 `Attack_1` Section 缺失，只保留后续段资源。
   验证首击是否仍会错误进入第 2 段或第 3 段。

### 11.2 风险2：输入可能被静默吞掉

1. 风险等级：
   中风险
2. 问题描述：
   当前 `HandleAbilityInputPressed` 中，只要扫描到同 `InputID` 的 Ability，就会把输入标记为“已处理”。
   但如果本次按键最终没有任何 Ability 成功激活，函数仍可能返回“已处理”。
3. 可能影响：
   会出现“玩家按了普攻，但角色没有任何动作，同时外层也没有明确报出未成功激活”的情况。
   这类问题在联调阶段会比较难定位，尤其容易和输入问题、资源问题、Tag 阻挡问题混淆。
4. 代码定位：
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
5. 建议验证方向：
   给角色打上 `State.CannotAttack`。
   给角色打上 `State.CannotInput`。
   制造 Ability 授予异常或资源配置异常。
   观察是否出现“输入被吃掉但没有清晰失败信号”的表现。

### 11.3 风险3：Ability 已结束但 Montage 可能仍在播放

1. 风险等级：
   中风险
2. 问题描述：
   当前普攻段落依赖 `UAbilityTask_PlayMontageAndWait` 驱动。
   从代码使用方式看，存在“Ability 已结束，但 Montage 仍继续播放”的潜在时序错位风险。
3. 可能影响：
   在当前只测自然播完时，这个问题可能不明显。
   但后续一旦接入打断、闪避打断、受击取消，或者外部主动结束 Ability，就可能出现状态已经收尾、动画却还在继续的错位问题。
4. 代码定位：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
   `UTwoHeartsGA_NormalAttackBase::StartSegmentPlayback`
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
   `UTwoHeartsGA_NormalAttackBase::EndAbility`
5. 建议验证方向：
   在普攻过程中尝试外部中断或取消 Ability。
   观察 Ability 结束后调试状态是否已清空。
   观察 Montage 是否仍残留播放。

### 11.4 风险4：当前初始化与授予链路对后续联机扩展存在隐患

1. 风险等级：
   中风险
2. 问题描述：
   当前 `AbilitySystem` 初始化主要放在 `BeginPlay`。
   默认 Ability 授予逻辑受 `HasAuthority()` 约束。
   对当前“单角色本地验证”基本够用，但对后续双人共斗、联机、重新 Possess、重生后重新建链等场景，鲁棒性仍需继续验证。
3. 可能影响：
   当前不是阻断单机普攻闭环的问题。
   但如果未来直接叠到双角色或联机链路上，可能出现客户端 Ability 不完整、输入链路不稳定、`ActorInfo` 初始化时机不一致等问题。
4. 代码定位：
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `AtwoheartsCharacter::BeginPlay`
   `AtwoheartsCharacter::InitializeAbilitySystem`
   `AtwoheartsCharacter::GrantDefaultCombatAbilities`
5. 建议验证方向：
   后续进入双角色、本地双控、联机或重新 Possess 场景时，重点复查：
   `ActorInfo` 初始化是否稳定。
   默认 Ability 是否稳定授予。
   输入是否仍能稳定进入 Ability 链路。

### 11.5 风险5：调试面板在段切换瞬间可能误导测试判断

1. 风险等级：
   低风险
2. 问题描述：
   当前调试状态在段切换过程中，会先被清成非攻击状态，再由下一段重新置回攻击状态。
   这不一定代表功能错误，但会让测试人员在观察 HUD 和事件顺序时产生误判。
3. 可能影响：
   在排查“第 2 段接不上”或“是否提前回待机”这类问题时，容易把段切换瞬间的状态抖动误判为真实功能异常。
4. 代码定位：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
   `UTwoHeartsGA_NormalAttackBase::EndAbility`
   `UTwoHeartsGA_NormalAttackBase::UpdateDebugState`
5. 建议验证方向：
   查看 `Recent Events` 时，不要只看某一帧的 `attacking=false`。
   需要结合 `SegmentFinished`、`AdvanceSegment`、`PlaySegment` 的先后顺序一起判断。

## 12. 如有必要的最小日志需求

### 12.1 建议项：输入分发层补一条最小结构化日志

1. 必要性判断：
   当前不是必须立刻加工具。
   但如果开发侧后续在联调时频繁遇到“输入按下但 Ability 没成功激活”的问题，这条日志会非常有价值。
2. 需求描述：
   建议只在输入分发层补一条最小结构化日志，不建议额外新增复杂测试工具。
3. 建议记录内容：
   本次按键扫描到了哪些同 `InputID` Ability。
   哪些 Ability 被尝试激活。
   最终成功激活的是哪一个。
   如果都失败，失败时是否存在阻挡 Tag、资源异常或未满足激活条件。
4. 主要用途：
   快速定位“输入被静默吞掉”。
   快速定位“首击为什么没有进入第 1 段”。
5. 当前结论：
   除这一条之外，当前阶段不建议额外提出新的测试工具需求。

## 13. 当前已发现的正式问题记录

### 13.1 问题1：普通攻击连段异常，第 2 段未正常表现

1. 问题等级：
   高优先级
2. 问题现象：
   当前进入游戏后，角色可以正常释放普通攻击。
   第一次点击普攻时，会正常播放第 1 段。
   但在第 1 段持续期间再次点击普攻，后续动作仍会再次播放第 1 段。
   连续点击普攻时，当前只明确观察到第 1 段和第 3 段，未正常观察到第 2 段。
3. 复现步骤：
   进入游戏。
   使用当前角色执行普通攻击。
   在第 1 段播放期间再次点击普攻。
   持续多次点击，观察后续段落表现。
4. 预期结果：
   第 1 段持续期间再次点击普攻后，应在第 1 段结束时稳定衔接到第 2 段。
   在第 2 段持续期间再次点击时，应进一步衔接到第 3 段。
   正常情况下应能稳定观察到完整的 1-2-3 连段表现。
5. 实际结果：
   当前未稳定观察到第 2 段。
   第 1 段期间再次点击后，后续动作仍可能重新播放第 1 段。
   连续点击时，仅见到第 1 段和第 3 段的表现。
6. 当前影响：
   当前普通攻击最核心的 1-2-3 连段闭环未通过验证。
   该问题会直接影响本阶段“普通攻击完全切换到 AbilitySystem”的验收。
7. 当前怀疑方向：
   第 1 段期间的再次输入，可能没有正确传递给当前激活中的普通攻击 Ability。
   第 1 段结束后，可能没有正确激活第 2 段 Ability。
   也不排除第 2 段 Ability 已激活，但第 2 段对应的 Montage Section 配置异常，导致视觉表现不像第 2 段。
8. 建议开发优先排查：
   当前段执行期间再次点击时，激活中的 Ability 是否真的收到了输入。
   第 1 段结束后，`Ability.NormalAttack.Segment2` 是否被成功激活。
   第 2 段实际使用的 Montage Section 是否正确指向 `Attack_2`，而不是错误落到其他 Section。

### 13.2 问题2：当前测试场景未看到普通攻击调试 HUD

1. 问题等级：
   中优先级
2. 问题现象：
   根据代码白盒 review，项目内存在普通攻击调试 HUD 面板逻辑。
   但当前进入游戏测试时，未在屏幕上看到普通攻击相关的调试界面。
3. 复现步骤：
   进入当前测试场景。
   使用当前角色进行普通攻击测试。
   观察屏幕上是否出现普通攻击调试面板。
4. 预期结果：
   如果当前测试方案依赖普通攻击调试 HUD 作为联调工具，则进入测试场景后应能看到对应调试界面。
   该界面应至少能提供：
   `attacking`
   `segment`
   `queued_next`
   `latest_section`
   `Last Failure`
   `Recent Events`
5. 实际结果：
   当前测试过程中未看到对应 HUD 界面。
6. 当前影响：
   当前测试人员无法直接通过屏幕调试面板观察：
   普攻当前段号；
   是否收到“下一段请求”；
   当前播放的 Section；
   最近失败原因与事件顺序。
   这会直接增加普通攻击连段问题的定位成本。
7. 当前怀疑方向：
   当前测试场景或当前 GameMode 可能没有挂载 `ATwoheartsDebugHUD`。
   也可能当前项目实际使用的是其他 HUD，导致普通攻击调试界面没有被绘制。
8. 建议开发优先排查：
   当前测试场景对应的 GameMode 是否已使用 `ATwoheartsDebugHUD`。
   当前测试链路是否预期依赖 HUD 面板进行联调。
   如果当前阶段不准备挂 HUD，是否至少需要提供可替代的日志观察方式。
