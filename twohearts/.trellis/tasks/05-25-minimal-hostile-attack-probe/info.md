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

## 已知限制

1. 本轮没有接正式 Montage Notify，而是使用 `C++` 阶段时序和 overlap 命中窗口；后续若进入更正式敌人攻击制作，可再替换为 Montage/Notify 驱动，但不必推翻当前玩家侧接收接口。
2. 当前命中只发送来袭/接触信号，不做正式伤害、硬直或格挡结果改写。
3. 当前默认资源采用直接内容引用，属于当前阶段为确保探针可立即放置与复现而接受的过渡实现；若后续敌人体系正式化，可再收束为蓝图/数据配置。

## 验证记录

1. 已执行 `Development Editor | Win64` 整编。
2. 结果：通过。
3. 当前未做 PIE 白盒跑关；需要进入编辑器把探针放入关卡后，结合 HUD 观察来袭信号。
4. 本机当前确认使用的构建入口：
   Unreal Engine：`G:\UE_5.6`
   MSBuild：`D:\VS2022\MSBuild\Current\Bin\MSBuild.exe`

## 对后续任务的直接价值

1. `05-25-player-hit-damage-minimum-loop` 现在可以直接消费 `UTwoHeartsHostileAttackReceiverComponent` 上的最近来袭信号或历史信号，不必再先造一套新的敌方攻击来源。
2. `05-25-basic-guard-implementation` 后续可以在同一入口上改写“命中”结果为“被格挡/未格挡”，而不是重做攻击探针。
