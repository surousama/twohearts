# 第二章基础战斗代码地图

## 用途

1. 这份文档只服务“快速定位第二章基础战斗代码”，不替代详细实现文档。
2. 主程序用它做阶段判断、拆 task、识别技术债。
3. 资深程序用它在开工前快速确认入口、数据流和真相源，再决定是否下钻源码。

## 当前适用范围

1. 普通攻击
2. 闪避
3. Guard
4. 玩家侧受击 / 伤害
5. 最小敌对攻击探针
6. 预输入
7. 调试 HUD 与白盒观察口径

## 当前阶段总判断

1. 第二章当前已经完成“玩家防守侧最小正式闭环”。
2. 当前仍未完成“玩家输出打到敌方”的正式闭环，以及受击 / Guard 的完整表现层。
3. 阅读代码时，不要再沿用“Guard 还没开始”“预输入还没做”的旧口径。

## 模块分层

### 1. 动作与公共语义层

职责：
统一记录当前动作类型、阶段、逻辑结束与打断关系，是普通攻击、闪避、Guard、受击状态的公共动作真相源。

核心文件：
* `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h`
* `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
* `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`

看这层时重点确认：
* 当前激活动作是谁
* 当前动作处于什么 phase
* 谁可以打断谁
* `LogicEnded` 与 `FinishAction` 是否正确收尾

### 2. 攻击描述层

职责：
统一承载攻击实例的最小正式描述，包括伤害值、受击类型、可格挡 / 可闪避、Guard 规则、Guard 结果和时序窗口。

核心文件：
* `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`

当前正式字段重点：
* `HitReactionType`
* `BaseDamage`
* `DamageMechanicTags`
* `bCanBeGuarded`
* `GuardMaxDistance`
* `GuardMaxHeightDifference`
* `GuardFacingHalfAngleDegrees`
* `GuardSuccessDisplacementResult`
* `GuardSuccessDamageResult`
* `GuardPartialDamageMultiplier`
* `bCanBeDodged`
* `TimingPhase`
* `TimingWindowName`

阅读注意：
* `AttackMetadata` 已经是正式输入层，不要再从 `Detail` 字符串反推攻击语义。
* 但当前“谁来稳定生产 metadata”还没有完全统一，尤其玩家普攻打敌方仍在补链路。

### 3. 玩家动作 Ability 层

职责：
承载玩家普通攻击、闪避、Guard 的正式 Ability 生命周期、动作 phase 进入、打断与输入消费。

核心文件：
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h`
* `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.cpp`

#### 普通攻击

职责：
* 正式承载 `1 / 2 / 3` 段普攻
* 管理 `Startup / Active / Recovery / LogicEnded`
* 消费预输入并推进连段
* 当前已能生成 `AttackMetadataTemplate`，但还未正式打到敌方

入口关键点：
* `BuildCurrentAttackMetadata()`
* `TryInterruptByAction()`
* `FinishSegment()`
* `AttemptDeferredNextSegmentActivation()`

#### 闪避

职责：
* 方向解析
* Montage 播放
* 无敌帧 / 冷却
* 被 Guard / 受击打断

入口关键点：
* `TryInterruptByAction()`
* 冷却与无敌时间控制

#### Guard

职责：
* Guard 生命周期
* 绑定 hostile signal
* 按攻击元数据做 Guard 判定
* 触发 `GuardRewritten` 与 `GuardOutcome`

入口关键点：
* `HandleHostileAttackSignalReceived()`
* `TryEvaluateGuardAgainstAttackSignal()`
* `ApplyGuardSuccessCooldown()`

阅读注意：
* 当前 Guard 已有规则与结算，不再是“只有输入窗口”
* 当前 Guard 表现仍主要是日志、HUD、`DrawDebug`

### 4. 玩家角色与输入桥接层

职责：
* 持有 `ASC`
* 授予默认战斗 Ability
* 转发输入
* 持有 Dodge / Guard / PreInput 配置
* 承担部分调试状态快照

核心文件：
* `Source/twohearts/twoheartsCharacter.h`
* `Source/twohearts/twoheartsCharacter.cpp`

关键配置入口：
* `FTwoHeartsDodgeConfig`
* `FTwoHeartsGuardConfig`
* `FTwoHeartsPreInputConfig`

阅读注意：
* `AtwoheartsCharacter` 当前是桥接层，不应重新退化成战斗状态总仓库。
* 看到大量 debug 字段时，要先区分它们是“观察字段”还是“长期真相源”。

### 5. 来袭接收、伤害、Guard 结算与受击状态层

职责：
* 接收 hostile signal
* 生成 `PlayerHitResult`
* 生成 `PlayerDamageResult`
* 记录 `GuardOutcome`
* 维护 `HitReactionState`

核心文件：
* `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
* `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`

当前正式结构：
* `FTwoHeartsHostileAttackSignal`
* `FTwoHeartsPlayerHitResult`
* `FTwoHeartsPlayerDamageResult`
* `FTwoHeartsGuardSettlementRequest`
* `FTwoHeartsGuardOutcome`
* `FTwoHeartsPlayerHitReactionState`

阅读这层时重点看三条链：
1. `ReceiveHostileAttackSignal() -> UpdatePlayerHitResultFromSignal()`
2. `PushPlayerHitResult() -> UpdatePlayerDamageResultFromHitResult()`
3. `UpdateHitReactionStateFromDamageResult() -> EnterHitReaction() -> FinishHitReaction()`

当前真相源判断：
* 玩家受击 / 伤害 / Guard 结算的当前真相源主要在这里
* 但生命值仍挂在这个组件上，属于后续应收口的中期技术债

### 6. 最小敌方攻击样本层

职责：
* 作为当前最小敌对攻击来源
* 构造一份完整的攻击元数据
* 按 `Startup -> HitWindow -> Recovery -> Finished` 广播给玩家 receiver

核心文件：
* `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.h`
* `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.cpp`

当前意义：
* 它既是最小敌方攻击样本，也是第二章很多白盒验证的主入口
* 不能把它误认为完整敌人框架

阅读重点：
* `InitializeCurrentAttackMetadata()`
* `BuildCurrentAttackMetadataSnapshot()`
* `BuildSignal()`
* `NotifyHitTargets()`

### 7. 调试与观察层

职责：
* 把动作、输入、来袭、受击、伤害、Guard 结算等信息统一暴露到 HUD
* 为白盒验收提供稳定观察口径

核心文件：
* `Source/twohearts/twoheartsDebugHUD.cpp`

当前可观察板块：
* Public Action Context
* Input Evaluation
* Dodge
* Guard
* Incoming Hostile Attack
* Player Hit Result
* Player Damage Result
* Guard Outcome
* Hit Reaction

阅读注意：
* HUD 是观察层，不是业务真相源。
* 如果 HUD 文案和运行表现矛盾，先回查对应组件，再判断是逻辑问题还是展示问题。

## 关键调用链

### 1. 敌方攻击打到玩家

`HostileAttackProbe.StartAttackStartup`
-> `BuildSignal`
-> `HostileAttackReceiver.ReceiveHostileAttackSignal`
-> `OnHostileAttackSignalReceived.Broadcast`
-> `Guard.TryEvaluateGuardAgainstAttackSignal`
-> `UpdatePlayerHitResultFromSignal`
-> `UpdatePlayerDamageResultFromHitResult`
-> `UpdateHitReactionStateFromDamageResult`

### 2. Guard 成功改写

`ReceiveHostileAttackSignal(AttackContact)`
-> `Guard.TryEvaluateGuardAgainstAttackSignal`
-> `RewriteLastPlayerHitResultForGuard`
-> `UpdatePlayerDamageResultFromHitResult`
-> `CommitGuardOutcome`
-> HUD / 日志观察

### 3. 玩家受击打断当前动作

`DamageApplied`
-> `EnterHitReaction`
-> `InterruptCurrentActionForHitReaction`
-> 当前动作 Ability `TryInterruptByAction(HitReaction)`
-> `CombatActionContext.FinishAction`

### 4. 普攻连段与预输入消费

`NormalAttack Input`
-> `HandleAbilityInputPressed`
-> `EvaluateCombatInput`
-> `ExecuteNow / BufferInput`
-> `NormalAttack Ability`
-> `FinishSegment`
-> `TryConsumeReservedCombatInput / TryTakeBufferedCombatInput`

## 当前真相源与过渡字段

### 当前优先相信

1. 当前动作 / phase / 结束原因：
   `UTwoHeartsCombatActionContextComponent`
2. 当前攻击语义：
   `FTwoHeartsAttackMetadata`
3. 玩家侧命中 / 伤害 / Guard 结算 / 受击状态：
   `UTwoHeartsHostileAttackReceiverComponent`
4. 输入缓存与输入评估：
   `AtwoheartsCharacter` + `CombatActionContext`

### 当前仍偏过渡

1. `HostileAttackReceiverComponent` 内部生命值承载
2. `AtwoheartsCharacter` 上的大量 debug runtime 字段
3. Guard 结果中的资源语义，当前仍以最小布尔与冷却为主
4. 普攻 `AttackMetadataTemplate` 目前还没有完整接上“打敌方”的正式消费侧

## 主程序使用方式

1. 先看“模块分层”和“关键调用链”
2. 再判断这次需求改的是哪一层
3. 再决定要不要拆 task，还是直接回写阶段文档 / spec
4. 若涉及长期结构判断，优先看“当前真相源与过渡字段”

## 资深程序使用方式

1. 先看这次任务落在哪一层
2. 先读本地图，再打开对应 `*.h / *.cpp`
3. 若观察问题，先查“关键调用链”上游与下游，不要一开始全仓库漫游
4. 若发现字段既像 debug 又像业务真相源，先回主程序确认是否需要收口
