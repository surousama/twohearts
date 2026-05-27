# 玩家受击与伤害最小闭环实施说明

## 当前结论

1. 本 task 已在玩家侧建立正式、最小、可观察的受击结果入口，不再只有“收到了 hostile signal”这一层。
2. 当前实现把“敌方攻击来袭信号”和“玩家侧受击结果”明确拆开：
   hostile probe 继续负责发信号；
   玩家侧 `UTwoHeartsHostileAttackReceiverComponent` 负责收束为正式结果。
3. 本轮没有扩散到完整血量、硬直、击退或受击表现系统，仍严格停留在 Guard 前置闭环范围内。

## 当前目标

1. 当最小敌对攻击探针命中玩家时，玩家侧产出一次明确、可读、可复用的受击结果。
2. 该结果至少覆盖：
   攻击来源；
   是否命中成立；
   结果时间；
   是否允许后续被 Guard 改写；
   基础结果类型。
3. 给后续 `05-25-basic-guard-implementation` 一个直接可接入的结果入口，而不是让 Guard 再造并行命中系统。

## 本轮明确不做

1. 不做正式 HP/扣血/死亡链路。
2. 不做正式受击硬直、击退、受击动画与音效。
3. 不做 Guard 输入与 Guard Ability 本体。
4. 不把玩家受击结果层过早泛化成完整 RPG 数值框架。

## 方案选择

1. 结果真相源优先放在 `UTwoHeartsHostileAttackReceiverComponent`，而不是继续堆在 `AtwoheartsCharacter` 上。
   原因：
   当前 hostile probe 的正式入口已经稳定对接到 receiver 组件；
   后续 Guard 只需要拦截/改写结果，不必回退到角色层重写信号接收；
   这更符合当前项目“Character 只保留角色入口与调试承载”的实现约束。
2. 本轮不新增独立“伤害组件”或数值系统，而是在现有 receiver 上加一层正式结果模型，保持最小闭环。
3. 调试观察优先复用已有 `twoheartsDebugHUD`，不另造一套面板。

## 本轮实际完成

1. 已在 `UTwoHeartsHostileAttackReceiverComponent` 中新增正式结果类型：
   `ETwoHeartsPlayerHitResultType`
2. 已新增玩家受击结果结构：
   `FTwoHeartsPlayerHitResult`
3. 已新增玩家侧结果历史、最近一次结果和结果广播：
   `GetLastPlayerHitResult()`
   `GetPlayerHitResultHistory()`
   `OnPlayerHitResultUpdated`
4. 已把 hostile signal 收束为正式玩家结果：
   `AttackStarted / HitWindowOpened`
   -> `PendingIncomingHit`
   `AttackContact`
   -> `HitConfirmed`
   `HitWindowClosed / AttackFinished`
   -> `HitExpired`
   非法时序
   -> `SignalInvalid`
5. 已增加最小 Guard 改写入口：
   `RewriteLastPlayerHitResultForGuard(...)`
6. 已在 `twoheartsDebugHUD` 中新增 `Player Hit Result` 区块，用于显示：
   结果类型；
   是否命中成立；
   是否仍允许被 Guard 改写；
   结果时间；
   来源 signal；
   detail。

## 当前结果语义口径

1. `PendingIncomingHit`
   表示敌方攻击已经进入玩家侧待结算状态，但尚未确认接触命中。
2. `HitConfirmed`
   表示 probe 的 `AttackContact` 已经到达玩家侧，这一结果当前被视为“可被后续 Guard 改写”的正式入口。
3. `HitExpired`
   表示本次攻击窗口已经结束，但没有形成玩家命中确认。
4. `SignalInvalid`
   表示收到了无法挂接到有效攻击实例的 signal，用于暴露时序异常，而不是默默吞掉。
5. `GuardRewritten`
   当前只作为预留结果类型；本轮没有接正式 Guard 逻辑，但 receiver 已提供改写入口。

## 计划代码落点与实际落点

1. 玩家受击结果真相源：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
2. 调试显示：
   `Source/twohearts/twoheartsDebugHUD.cpp`
3. 本轮没有新增角色层状态机，也没有把结果散落到 `AtwoheartsCharacter` 临时字段中。

## 对后续 Guard 的直接承载方式

1. 后续 Guard 不需要重新定义“什么叫玩家这次被打到了”。
2. Guard 只需要在合适窗口拦截或改写 `HitConfirmed` 结果即可。
3. 当前 receiver 已通过 `bCanBeRewrittenByGuard` 和 `RewriteLastPlayerHitResultForGuard(...)` 预留最小改写语义。

## 已知限制

1. 当前结果仍是“最小受击结果层”，不是完整伤害系统。
2. 本轮没有接正式血量字段，因此 `HitConfirmed` 代表规则层命中成立，不等于已经扣血。
3. 当前 Guard 改写入口只做了最小 API 预留，还没有和输入、动作窗口或 Ability 生命周期联动。
4. 当前没有新增自动化测试或 PIE 脚本验证；本轮完成的是代码编译闭环和 HUD/日志可观察闭环。

## 验证记录

1. 已执行 UnrealBuildTool 编译：
   `dotnet H:\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll twoheartsEditor Win64 Development -Project="H:\twohearts\twohearts\twohearts.uproject" -WaitMutex -NoHotReload`
2. 结果：通过。
3. 已完成两轮 PIE 白盒验证，使用对象：
   `BP_HostileAttackProbe_SlowSingle`
4. 第一轮 PIE 验证结论：
   命中主链路与未命中主链路本身都成立；
   但在最终结果已落成后，后续正常到来的 `HitWindowClosed / AttackFinished` 被误判成了 `SignalInvalid`；
   因此第一轮结论是“主判定链路基本正确，但收尾信号处理存在缺陷，不可直接算通过”。
5. 已根据第一轮日志修复玩家侧收尾污染问题：
   当某个攻击实例已经落成最终结果 `HitConfirmed / HitExpired / GuardRewritten` 后，
   其后续晚到的生命周期收尾信号不再覆盖结果，
   而是明确记录为：
   `stage=LateLifecycleSignalIgnored`
6. 第二轮 PIE 验证结论：
   命中轮：
   `AttackStarted -> PendingIncomingHit -> HitWindowOpened -> AttackContact -> HitConfirmed`
   成立；
   `HitWindowClosed / AttackFinished` 不再污染最终结果。
7. 第二轮未命中轮结论：
   `AttackStarted -> PendingIncomingHit -> HitWindowOpened -> HitWindowClosed -> HitExpired`
   成立；
   后续 `AttackFinished` 不再污染最终结果。
8. 第二轮日志中的关键验收事实：
   `HostileProbe_1` 命中后出现 `HitConfirmed`；
   同一实例后续 `HitWindowClosed` 与 `AttackFinished` 被记录为 `LateLifecycleSignalIgnored`；
   `HostileProbe_2` 未命中后出现 `HitExpired`；
   同一实例后续 `AttackFinished` 被记录为 `LateLifecycleSignalIgnored`。
9. 因此本 task 当前白盒验收结论为：
   通过。
10. 当前玩家受击最小闭环已满足后续 Guard 接入前提：
    玩家侧存在正式结果入口；
    能稳定区分命中成立与窗口过期；
    结果不会被正常收尾信号二次污染。

## 对后续任务的直接价值

1. `05-25-basic-guard-implementation` 现在可以直接以 `HitConfirmed` 为命中入口，而不是再发明一套 guard-only 命中判定。
2. `05-25-guard-feedback-and-whitebox-validation` 后续可以直接观察 HUD 里的 `Player Hit Result` 区块，确认 Guard 是否成功改写了玩家侧结果。
