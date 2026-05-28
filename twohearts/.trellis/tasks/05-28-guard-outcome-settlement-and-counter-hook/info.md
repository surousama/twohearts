# 技术实现记录

## 本轮实现目标

在现有 `GuardRewritten / GuardBlocked` 骨架之上，补齐格挡成功后的最小正式结算：

1. 位移结果分类
2. 伤害结果分类
3. 资源 / 冷却语义
4. 反击接口预留与可观察性

## 主要实现落点

- `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
  - 新增 Guard 结算相关枚举与攻击侧配置字段：位移分类、伤害分类、部分承伤倍率。
- `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  - 新增 `FTwoHeartsGuardSettlementRequest` 与 `FTwoHeartsGuardOutcome`。
  - Receiver 在 Guard 改写正式落地后，统一沉淀 Guard Outcome，并保留一个最小的后续技能读取点：
    - `DoesLastGuardEnableFollowUpAbility()`
  - Guard 成功后的伤害结果不再一律只有 `GuardBlocked`，而是按攻击元数据支持：
    - `FullyBlocked`
    - `PartialDamageTaken`
    - `PenetrationFailed`
- `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  - 新增 `Cooldown.Guard` 激活阻塞。
  - 本轮明确语义：Guard 仅在“成功格挡”时建立冷却；不在尝试时扣，不在失败时扣。
- `Source/twohearts/twoheartsCharacter.h`
  - 新增 `GuardSuccessCooldownSeconds` 配置。
- `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  - 为白盒样本攻击新增 Guard Outcome 配置入口。
- `Source/twohearts/twoheartsDebugHUD.cpp`
  - 新增 Guard 冷却可视化与 `Guard Outcome` 面板。

## 当前正式语义

### 位移结果

当前先以结果分类承载，不直接驱动物理位移。默认 hostile probe 样本为 `AttackerPushedBack`，但这仍属于“规则结果分类 + 调试可视化”，不是最终表现位移实现。

### 伤害结果

Guard 成功后，正式区分：

- `FullyBlocked`
- `PartialDamageTaken`
- `PenetrationFailed`

其中：

- `FullyBlocked` / `PenetrationFailed` 当前都不产生最终伤害
- `PartialDamageTaken` 使用攻击元数据里的 `GuardPartialDamageMultiplier`

### 资源 / 冷却

本轮明确口径：

- 资源：当前阶段**没有独立 Guard 资源系统**，因此 Guard 成功后 `resource_consume=false`、`resource_refund=false`
- 冷却：当前阶段采用**成功格挡触发冷却**，冷却标签为 `Cooldown.Guard`
- 配置入口：`FTwoHeartsGuardConfig::GuardSuccessCooldownSeconds`

### 反击接口预留

当前仅保留一个最小读取点：

- `DoesLastGuardEnableFollowUpAbility()`

它只表达“最近一次正式 Guard 结果是否允许后续技能读取”，不再承载额外生命周期、消费或打断语义。

## 已知边界

1. 位移结果当前仍是“分类 / 调试 / 规则语义”，未驱动真实推退表现。
2. 反击当前只保留一个最小后续技能读取点，未定义额外生命周期、消费或打断语义。
3. 当前白盒 hostile probe 默认仍偏向最小验证样本，后续可继续扩更多 outcome 组合做回归。

## 白盒 Review 问题单（2026-05-28）

### 1. [P1] Guard 改写未与当前攻击实例强绑定，存在误改写上一击风险

- **结论**：`UTwoHeartsHostileAttackReceiverComponent::RewriteLastPlayerHitResultForGuard()` 的即时改写分支，仅校验 `bHasPlayerHitResult` 与 `LastPlayerHitResult.bCanBeRewrittenByGuard`，未校验 `AttackInstanceName` 是否与本次 `SettlementRequest` 对应。
- **影响范围**：`AttackContact` 前序 signal 缺失、时序异常、不同敌人实现未严格遵循 hostile probe 样本时，可能把上一击的 `LastPlayerHitResult` 误改写为本次 Guard 成功。
- **放大后果**：该改写后续会继续驱动伤害回滚/重算与 `GuardOutcome` 沉淀，因此不是单点观测偏差，而是会扩散成错误血量与错误 outcome。 
- **白盒证据**：
  - `ReceiveHostileAttackSignal()` 在 `AttackContact` 分支里先 `Broadcast` 给 Guard，再 `UpdatePlayerHitResultFromSignal()`。
  - `RewriteLastPlayerHitResultForGuard()` 的即时改写路径未把 `SettlementRequest.AttackInstanceName` 与 `LastPlayerHitResult.AttackInstanceName` 做强匹配。
- **建议修复**：即时改写分支应显式校验当前攻击实例；如实例不匹配，应拒绝改写或转为仅记录待处理状态。
- **建议回归**：补 `contact-only`、漏发 `AttackStarted`、漏发 `HitWindowOpened`、双攻击实例交错四组异常时序用例，确认不会改写上一击结果。

### 2. [P2] `source_hit` / `source_damage` 命名与实际含义不符，易误导观察结论

- **结论**：`CommitGuardOutcome()` 中写入的 `SourceHitResultType` 与 `SourcePlayerDamageResultType` 实际取自改写后的 `HitResult.ResultType` / `LastPlayerDamageResult.ResultType`，并非真正的“改写前原始结果”；但 HUD 侧以 `source_hit` / `source_damage` 名义直接展示，语义与字段名不一致。
- **影响范围**：测试与程序在做白盒分析时，无法仅靠当前 HUD 明确区分“命中前即被 Guard 改写”与“先命中扣血，后回滚为 Guard 结果”两条路径，观察面会产生误导。
- **白盒证据**：
  - `CommitGuardOutcome()` 里 `GuardOutcome.SourceHitResultType = HitResult.ResultType`，此时 `HitResult` 已可能是 `GuardRewritten`。
  - `twoheartsDebugHUD.cpp` 直接输出 `source_hit=%s   source_damage=%s`，容易被误解为“原始输入结果”。
- **建议修复**：二选一即可：
  - 真正保留改写前快照，新增 `PreRewriteHitResultType` / `PreRewriteDamageResultType`；或
  - 承认当前字段是最终结算结果，将 HUD 与结构体字段重命名为 `resolved_*`，避免伪 source 语义。
- **建议回归**：分别构造“命中前改写”和“先扣血后回滚”样本，确认 HUD 输出能够稳定区分两种路径，否则继续补充观测字段。

## 白盒问题修复补充（2026-05-28）

1. `RewriteLastPlayerHitResultForGuard()` 现已要求 `SettlementRequest.AttackInstanceName` 与目标攻击实例强匹配：
   - 立即改写分支只允许改写同 `AttackInstanceName` 的当前可重写命中结果；
   - 若当前仅存在 pending attack，则只允许把改写请求排队到同实例；
   - 若实例缺失或不匹配，receiver 直接拒绝并记日志，不再允许误改写上一击。
2. Guard Outcome 观察字段统一改名为 `resolved_hit / resolved_damage`，明确它们表达的是当前最终结算结果，而不是“改写前原始结果”。
