# 最小敌对攻击探针实施说明

## 当前结论

1. 当前任务在资深程序视角下可以直接开工，不需要再回到上游补 PRD。
2. 本轮采用“最小 humanoid 攻击探针”方案，而不是纯按钮式一次性测试脚本。
3. 探针优先做成可放入关卡、可重复触发、可观察时序、可向玩家侧暴露来袭事件的正式调试入口。

## 当前目标

1. 提供一个最小敌对攻击承载体，能在关卡中稳定复现一次攻击流程。
2. 明确拆出攻击的开始、命中窗口、结束和复位。
3. 以 `C++` 为真相源，把“来袭已开始 / 命中窗口已开启 / 命中已发生或可发生”暴露给后续玩家受击与格挡任务。

## 本轮明确不做

1. 不做完整敌人 AI、寻路、索敌、追击。
2. 不做正式伤害数值、扣血、死亡。
3. 不做正式 Guard 输入、Guard Ability、格挡成功/失败结算。
4. 不为未来所有敌人一次性抽完整通用攻击框架。

## 方案选择

1. 承载体优先使用最小 `ACharacter` 或基于角色骨骼的最小敌对角色，而不是裸 `AActor`。
   原因：
   现有项目已经具备可复用的人形动画资源；
   后续若要继续接入受击、格挡和命中朝向，角色承载比一次性 Actor 更容易复用；
   同时仍可保持“无 AI、只做最小攻击循环”的低复杂度。
2. 攻击时序优先采用“单段 Montage / 动画 + 明确命中时窗”。
3. 命中窗口的正式真相源优先放在 `C++` 攻击探针组件或角色逻辑中；动画 Notify 仅用于驱动时机，不直接成为长期规则真相源。
4. 攻击提示本轮优先采用调试可读手段：
   屏幕调试输出；
   `UE_LOG`；
   必要时补充调试形状。

## 资源与配置记录

1. 当前已确认项目内存在可复用攻击动画资源：
   `Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.uasset`
   `Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.uasset`
   `Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.uasset`
2. 当前已确认项目内存在可复用受击动画资源，但本 task 暂不接正式受击播放：
   `Content/Characters/Mannequins/Anims/Rifle/HitReact/*`
3. 若实现过程中需要新增或补齐以下资源/配置，必须继续回写到本文档：
   敌对角色蓝图或数据资源；
   攻击 Montage；
   Notify 时机配置；
   测试关卡放置方式；
   调试 HUD/日志开关。

## 计划代码落点

1. 新增最小敌对攻击承载代码，优先放在 `Source/twohearts/TwoHearts/Combat/` 及其子目录。
2. 若需要对玩家侧暴露统一来袭事件，优先补在可被后续复用的战斗层数据结构或组件中。
3. 若需要最小调试显示，优先复用已有 `twoheartsDebugHUD` 或角色调试输出体系，不另造一套独立调试入口。

## 事件出口要求

1. 至少要有一个可被后续 `05-25-player-hit-damage-minimum-loop` 直接复用的结构化出口。
2. 该出口最少应包含：
   攻击来源；
   攻击开始时间或命中窗口时间；
   攻击方向或来源位置；
   当前是否处于命中窗口；
   本次攻击实例标识或可读名字。
3. 即使本轮只先打通日志和调试绘制，也要确保后续玩家侧无需推翻接口重做。

## 风险与约束

1. 当前任务是“受击与格挡前置闭环”，不是正式敌人系统；若实现开始膨胀，应优先收窄为单段攻击、单一触发方式、单一命中窗口。
2. 不允许把测试辅助逻辑伪装成长期正式结构；若存在过渡实现，必须在代码和文档里标注用途。
3. 蓝图和动画资源可以承担表现与时机配置，但命中窗口、状态切换和事件暴露仍以 `C++` 为准。

## 开工前最终判断

1. 当前上下文已足够支持直接进入实现。
2. 若后续唯一缺口变成“具体使用哪一个动画资源或 Notify 时点”，优先由程序按现有资源做最小可运行配置，不再因资源选择本身阻塞开工。

## 本轮实际完成

1. 已新增最小敌对攻击探针角色：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.h`
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.cpp`
2. 已新增玩家侧来袭接收组件：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
3. 已把玩家角色默认接上 `UTwoHeartsHostileAttackReceiverComponent`，后续玩家受击与格挡任务可直接复用该入口。
4. 已把 HUD 补到可显示最近一次来袭信号，便于本地白盒观察攻击开始、命中窗口和接触事件。
5. 已为敌对攻击探针补充专用 Gameplay Tag 与动作类型：
   `Ability.HostileAttackProbe`
   `State.Action.HostileAttackProbe`
   `ETwoHeartsCombatActionType::HostileAttackProbe`

## 当前实现口径

1. 探针承载体使用最小 `ACharacter`，默认直接硬引用项目内 Manny 网格与 unarmed 动画资源，避免这轮再额外手工配蓝图才能跑通。
2. 攻击流程使用 `C++` 定时阶段驱动：
   `Startup -> Active(HitWindow) -> Recovery -> Finish`
3. 敌对探针在攻击开始、命中窗口开启/关闭、接触发生、攻击结束时，都会向目标身上的 `UTwoHeartsHostileAttackReceiverComponent` 发送结构化信号。
4. 命中检测本轮使用前方 `Sphere` overlap，属于当前阶段允许的最小正式探针，不是长期完整敌人攻击框架。

## 默认资源与触发方式

1. 默认网格：
   `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple`
2. 默认待机动画：
   `/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle`
3. 默认攻击动画：
   `/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01`
4. 默认触发方式：
   玩家进入 TriggerSphere 后自动触发；
   若玩家留在范围内，探针可按冷却重复攻击；
   也可手动调用 `TriggerProbeAttack()`。
5. 当前已提供给策划直接使用的基础 Blueprint：
   `/Game/Characters/HostileAttackProbe/BP_HostileAttackProbe`
6. 当前已提供两个策划预设 Blueprint：
   `/Game/Characters/HostileAttackProbe/BP_HostileAttackProbe_SlowSingle`
   `/Game/Characters/HostileAttackProbe/BP_HostileAttackProbe_LoopPressure`

## 当前默认参数

1. Trigger 半径：`220.0`
2. 攻击前向距离：`140.0`
3. 攻击判定半径：`75.0`
4. Startup 时间：`0.5s`
5. HitWindow 时间：`0.15s`
6. Recovery 时间：`0.45s`
7. 重复攻击冷却：`0.8s`
8. 默认自动触发：开启
9. 默认目标停留时循环攻击：开启
10. 默认屏幕调试输出：开启
11. 默认调试形状绘制：开启

## 当前代码级配置事实

1. 探针默认使用 `AnimationSingleNode` 模式直接播放动画资源，不依赖额外 AnimBP 配置即可进入首轮验证。
2. Probe 网格默认使用：
   `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple`
3. 攻击命中判定体是前方 `HitSphereComponent`，不是武器骨骼碰撞，也不是 Montage Notify 驱动的 Trace。
4. 玩家侧接收入口由 `AtwoheartsCharacter` 默认创建：
   `UTwoHeartsHostileAttackReceiverComponent`
5. HUD 当前会显示最近一次来袭信号的：
   信号类型；
   攻击实例名；
   是否处于命中窗口；
   是否发生接触；
   来源 actor；
   攻击方向；
   detail。
6. 基础 BP 与两个策划预设 BP 都继承自同一 `C++` 父类，因此以下可调项会继续暴露在 Blueprint 默认值与关卡实例 Details 中：
   `TriggerRadius`
   `AttackReach`
   `AttackRadius`
   `StartupSeconds`
   `HitWindowSeconds`
   `RecoverySeconds`
   `RepeatCooldownSeconds`
   `bAutoTriggerWhenTargetEntersRange`
   `bLoopAttackWhileTargetStaysInRange`
   `bEnableScreenDebugOutput`
   `bDrawDebugShapes`

## 策划侧 PIE 配置步骤

1. 打开用于白盒验证的测试关卡，确认当前关卡使用项目默认 `GameMode`，这样 `ATwoheartsDebugHUD` 会自动生效，不需要策划额外切 HUD。
2. 策划侧默认不要直接使用 `C++` 类；优先从内容浏览器拖 Blueprint 资产进关卡。
3. 当前推荐使用顺序是：
   首选 `BP_HostileAttackProbe_SlowSingle` 做单次白盒观察；
   首选 `BP_HostileAttackProbe_LoopPressure` 做格挡节奏与持续压迫测试；
   只有需要自定义一整套新参数口径时，再使用基础版 `BP_HostileAttackProbe`。
4. 首次放置时，把 Probe 摆在玩家出生点正前方，建议和玩家相距 `120` 到 `180` Unreal Units，且 Probe 正面朝向玩家，先保证攻击前方判定球能够覆盖玩家站位。
5. 当前玩家角色已默认挂上 `UTwoHeartsHostileAttackReceiverComponent`，所以只要 PIE 使用的是现有主角，不需要策划再手动补任何接收组件。
6. 选中 Probe 后，在 Details 面板优先检查以下分组：
   `Combat|Hostile Attack Probe|Animation`
   `Combat|Hostile Attack Probe`
   `Combat|Hostile Attack Probe|Debug`
7. 若只是进入首轮验证，优先保持默认动画资源不改：
   IdleAnimation = `/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle`
   AttackAnimation = `/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01`
8. 若 Probe 放下后朝向或判定看起来不对，先通过旋转 Actor 修正整体朝向；只有在确认朝向正确但还是打不到人时，再调 `AttackReach` 和 `AttackRadius`，不要一开始就同时改很多参数。

## 当前策划预设说明

1. `BP_HostileAttackProbe`
   这是基础承载 Blueprint，参数基本跟随 `C++` 默认值，适合技术验证或做新的策划衍生版本。
2. `BP_HostileAttackProbe_SlowSingle`
   这是给策划做单次白盒观察的预设版本；
   默认更长前摇；
   默认不循环；
   更适合先观察 `AttackStarted -> HitWindowOpened -> AttackFinished` 的完整单轮时序。
3. `BP_HostileAttackProbe_LoopPressure`
   这是给策划做持续压迫与格挡节奏测试的预设版本；
   默认前摇更短；
   默认会持续循环攻击；
   默认攻击判定略大，减少“刚好擦边没测到”的白盒噪音。
4. 若没有明确新需求，策划不建议从基础版手工起配；优先复制或直接使用已有预设，能减少实例参数飘散。

## 建议的首轮策划参数

1. 基线验证配置：
   `TriggerRadius = 220`
   `AttackReach = 140`
   `AttackRadius = 75`
   `StartupSeconds = 0.5`
   `HitWindowSeconds = 0.15`
   `RecoverySeconds = 0.45`
   `RepeatCooldownSeconds = 0.8`
   `bAutoTriggerWhenTargetEntersRange = true`
   `bLoopAttackWhileTargetStaysInRange = true`
   `bEnableScreenDebugOutput = true`
   `bDrawDebugShapes = true`
2. 若策划想先看单次攻击，而不是角色站在原地被反复打，先只改两项：
   `bLoopAttackWhileTargetStaysInRange = false`
   保持 `bAutoTriggerWhenTargetEntersRange = true`
3. 若策划想测试“靠近不自动开打，由外部手动触发”，使用以下组合：
   `bAutoTriggerWhenTargetEntersRange = false`
   `bLoopAttackWhileTargetStaysInRange = false`
   然后在 Blueprint 或 Level Blueprint 里手动调用 `TriggerProbeAttack()`
4. 若策划想先把可读前摇拉长，便于观察格挡窗口，优先只增加 `StartupSeconds`，建议先调到 `0.7` 或 `0.8`，不要先动命中半径。
5. 若策划想提高“明明碰到了却没打中”的容错，优先小幅增加：
   `AttackReach` 每次加 `20`
   `AttackRadius` 每次加 `10`
   每次只改一项后重新 PIE 观察。

## PIE 白盒操作步骤

1. 进入 PIE 前，先确认玩家出生后会出现在 Probe 的触发圈附近，否则本轮看不到自动攻击。
2. 点击 Play 进入 PIE 后，把玩家走进 Probe 周围的青色 Trigger 球范围。
3. 若使用默认自动触发配置，预期会立即进入一次完整攻击流程，不需要额外按键。
4. 观察屏幕上的 Debug HUD，重点看 `Incoming Hostile Attack` 区块。
5. 一次完整攻击的推荐观察顺序是：
   `AttackStarted`
   `HitWindowOpened`
   `AttackContact`（若命中到玩家）
   `HitWindowClosed`
   `AttackFinished`
6. 如果当前站位没有进入命中球，但已经进了 Trigger 球，那么你通常会看到：
   有 `AttackStarted`
   有 `HitWindowOpened`
   但没有 `AttackContact`
   这说明配置已经跑通，只是攻击判定没覆盖到玩家。
7. 如果玩家持续留在 Trigger 范围内，且 `bLoopAttackWhileTargetStaysInRange = true`，那么 Probe 会在一次攻击结束并经过 `RepeatCooldownSeconds` 后再次攻击；这就是后续格挡节奏测试的默认循环入口。
8. 如果你只想观察单轮信号，攻击结束后把玩家拉出 Trigger 球，或者直接把循环开关关掉再 PIE。

## PIE 验收口径

1. HUD 中 `Incoming Hostile Attack` 不再显示 `no_hostile_attack_signal_yet`，说明玩家已成功收到至少一次来袭信号。
2. `type` 字段能按时序变化，说明 Probe 的阶段流转正常，不是只打出了一次静态 overlap。
3. `attack` 字段会显示类似 `HostileProbe_1`、`HostileProbe_2` 的实例名，说明本次攻击实例有被正确编号，后续可以用于格挡或受击调试对齐。
4. `hit_window=YES` 时对应的就是当前允许命中的窗口；若此时玩家身处前方攻击球内，应该同时或随后看到 `contact=YES`。
5. 屏幕调试与场景调试球都能看到时，说明当前适合策划白盒调参；如果这些信息太吵，再进入收口阶段时再逐项关闭 Debug 开关。

## 常见策划调参建议

1. 现阶段优先把 Probe 当成“攻击样本发生器”，不要急着把它包装成完整敌人；能稳定复现时序，比外观完整更重要。
2. 想做“更容易格挡”的版本，优先从 `BP_HostileAttackProbe_SlowSingle` 开始，再继续加大 `StartupSeconds`；不要每次都从基础版重新手调。
3. 想做“更有压迫感”的版本，优先从 `BP_HostileAttackProbe_LoopPressure` 开始，再缩短 `StartupSeconds` 或增大 `AttackReach`，但要同步确认玩家是否还有读招空间。
4. 如果需要多套白盒方案，建议直接复制现有预设 Blueprint 再改，例如从 `BP_HostileAttackProbe_SlowSingle` 或 `BP_HostileAttackProbe_LoopPressure` 派生，而不是让每个关卡实例都手改一遍。
5. 当前玩家侧接口已经固定在 `UTwoHeartsHostileAttackReceiverComponent`，所以策划调参时应尽量只动 Probe 侧参数，不要改玩家收信号的结构。

## 手动触发配置补充

1. 若要给策划一个“按键触发一次 Probe 攻击”的验证入口，推荐创建 Probe 的 Blueprint 子类，而不是改 `C++` 默认行为。
2. 在 Level Blueprint 中保存该 Probe 实例引用，绑定一个临时输入事件后调用 `TriggerProbeAttack()` 即可。
3. 如果要反复切回默认自动攻击验证，直接恢复：
   `bAutoTriggerWhenTargetEntersRange = true`
   `bLoopAttackWhileTargetStaysInRange = true`
4. 若 Probe 被手动测试打乱状态，可直接调用 `ResetProbeToIdle()`，让它清掉阶段计时器并回到 Idle。

## 已知限制

1. 本轮没有接正式 Montage Notify，而是使用 `C++` 阶段时序和 overlap 命中窗口；后续若进入更正式敌人攻击制作，可再替换为 Montage/Notify 驱动，但不必推翻当前玩家侧接收接口。
2. 当前命中只发送来袭/接触信号，不做正式伤害、硬直或格挡结果改写。
3. 当前默认资源采用直接内容引用，属于当前阶段为确保探针可立即放置与复现而接受的过渡实现；若后续敌人体系正式化，可再收束为蓝图/数据配置。
4. 当前仍主要依赖日志、HUD 与调试绘制做白盒验证，还没有上升到正式敌人调试面板或自动化测试。

## 验证记录

1. 已执行 `Development Editor | Win64` 整编。
2. 结果：通过。
3. 已完成 PIE 白盒验证，并实际放置策划侧推荐预设 `BP_HostileAttackProbe_SlowSingle` 进行多轮观察。
4. 已验证“命中命中球”分支：
   Probe 按顺序输出 `AttackStarted -> HitWindowOpened -> AttackContact -> HitWindowClosed -> AttackFinished`；
   玩家侧 `UTwoHeartsHostileAttackReceiverComponent` 同步收到完整来袭与接触信号。
5. 已验证“进入 Trigger 但未进入命中球”分支：
   Probe 仍会正常输出 `AttackStarted -> HitWindowOpened -> HitWindowClosed -> AttackFinished`；
   日志会明确出现 `ContactMiss`，说明探针与命中窗口正常，只是玩家不在命中范围内。
6. 已修复并验证一个收尾边界：
   玩家若在攻击尚未结束前离开 TriggerSphere，
   Probe 仍会基于本轮锁定目标补发 `AttackFinished`，
   不再因为 `CurrentTargetActor` 被清空而让玩家侧丢失结束信号。
7. 白盒验证当前主要依赖：
   `Saved/Logs/twohearts.log` 中的 `[HostileAttackProbe]` 与 `[HostileAttackSignal]` 日志；
   屏幕 Debug HUD 中的 `Incoming Hostile Attack` 区块。
8. 本机当前确认使用的构建入口：
   Unreal Engine：`G:\UE_5.6`
   MSBuild：`D:\VS2022\MSBuild\Current\Bin\MSBuild.exe`

## 对后续任务的直接价值

1. `05-25-player-hit-damage-minimum-loop` 现在可以直接消费 `UTwoHeartsHostileAttackReceiverComponent` 上的最近来袭信号或历史信号，不必再先造一套新的敌方攻击来源。
2. `05-25-basic-guard-implementation` 后续可以在同一入口上改写“命中”结果为“被格挡/未格挡”，而不是重做攻击探针。
