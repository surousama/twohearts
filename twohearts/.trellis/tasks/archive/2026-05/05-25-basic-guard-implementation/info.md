# 基础格挡正式实现进度记录

## 2026-05-27 当前进度

### 本轮已完成

1. 已将 `Guard` 正式接入玩家输入链、默认 Ability 授予链和公共动作上下文。
2. 已新增 `Ability.Guard` Tag 与 `UTwoHeartsGA_Guard`，不再只停留在输入枚举和 `State.Action.Guard` 预留。
3. 已按当前确认口径实现基础 Guard 生命周期：
   `Startup -> Active -> Recovery -> LogicEnded`
4. 已实现基础 Guard 判定窗口：
   当前为单击短窗口；
   不支持长按维持；
   但已经预留输入释放观察口和 `HoldReserved` 扩展枚举。
5. 已将 Guard 成功改写接到现有 `UTwoHeartsHostileAttackReceiverComponent`：
   当最小敌对攻击探针把玩家结果推进到 `HitConfirmed` 且允许改写时，
   若 Guard 正处于活动窗口，则会将结果改写为 `GuardRewritten`。
6. 已按当前确认口径允许 Guard 从当前玩家已有动作状态尝试进入，并补齐以下打断关系：
   Guard 可打断普通攻击；
   Guard 可打断基础 Dodge；
   普攻与 Dodge 的动作上下文收尾已补成“只收自己的上下文”，避免被 Guard 抢占时互相踩状态。
7. 已扩展本地调试 HUD，当前可直接观察：
   Guard 是否激活；
   Guard 窗口是否开启；
   当前 Guard phase；
   最近一次 Guard 事件；
   玩家受击结果是否已被改写。

### 当前实现边界

1. 本轮只保证对 `05-25-minimal-hostile-attack-probe` 提供的最小敌对攻击探针生效。
2. 本轮未实现长按维持 Guard。
3. 本轮未实现完美格挡、Parry、反击派生、耐力条、防御角度和完整防御体系。
4. 本轮未接正式 Guard 动画资源，当前以 `C++` 生命周期、定时窗口和调试可观察性为主。

### 主要代码落点

1. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h`
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.cpp`
3. `Source/twohearts/twoheartsCharacter.h`
4. `Source/twohearts/twoheartsCharacter.cpp`
5. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
6. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
7. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
8. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
9. `Source/twohearts/twoheartsDebugHUD.cpp`

### 已完成验证

1. 已成功通过本地 Unreal 编译：
   `Build.bat twoheartsEditor Win64 Development -Project=G:\twohearts\twohearts\twohearts.uproject -WaitMutex -NoHotReloadFromIDE`
2. 已完成 PIE 白盒跑测，并确认以下关键结论：
   Guard 可在空闲态稳定进入；
   Guard 可在普通攻击 `Recovery` 成功打断；
   Guard 在 `Startup / Active` 不再越权打断普通攻击；
   Guard 可将最小敌对攻击探针的 `HitConfirmed` 改写为 `GuardRewritten`。
3. 已通过日志确认 Guard 改写闭环实际发生：
   `AttackContact -> HitConfirmed -> GuardRewrite -> GuardRewritten -> GuardRewriteSuccess`
4. 为便于调试验证，曾临时把 Guard 活动窗口放大到 `1.00s`；
   在确认改写链路稳定命中后，当前已收回到更紧的临时值 `0.45s`。

### 尚未完成

1. 尚未验证 `0.45s` 窗口下的最终手感是否满足后续设计预期。
2. 尚未补 `05-25-guard-feedback-and-whitebox-validation` 所需的表现与验收结论。

### 补充说明

1. 编译过程中顺手修复了一个现有构建阻塞：
   `TwoHeartsHostileAttackReceiverComponent.cpp` 与 `TwoHeartsHostileAttackProbeCharacter.cpp` 的本地 `LexToString` 命名在构建条件下发生冲突；
   同时修正了 `HostileAttackReceiver` 内一处旧的格式化字符串问题。
2. 当前 `task.json` 的变更主要来自本轮 `task.py start` 将任务切到 `in_progress`。
3. 本轮额外修正了 Guard 成功改写后的伴随状态与调试显示：
   `GuardRewritten` 不再保留 `bHitConfirmed=true`；
   HUD 可将 Guard 成功与普通命中明确区分。
4. 本轮额外收紧了 Guard 打断白名单：
   当前只允许从无动作态进入；
   或在普通攻击 `Recovery / LogicEnded`；
   或在当前基础 Dodge 中进入。
