# 格挡反馈与白盒验收记录（2026-05-27）

## 本轮范围

1. 以资深测试工程师视角，对 `05-25-guard-feedback-and-whitebox-validation` 的最小可读反馈和白盒验收口径进行整理。
2. 本轮证据来源包括：
   `Saved/CombatDebug/normal-attack-debug.log`
   `Saved/Logs/twohearts.log`
   当前 Guard 相关 C++ 代码与调试入口。
3. 本轮不包含终版动作、特效、音效和镜头表现验收。

## 观察入口

1. HUD / Debug Overlay：
   Guard Active；
   Guard Window；
   Guard Phase；
   Last Guard Event；
   Last Player Hit Result。
2. 结构化日志：
   `GuardActivate`
   `GuardWindowOpened`
   `GuardWindowClosed`
   `GuardFinished`
   `AttackContact`
   `HitConfirmed`
   `GuardRewrite`
   `GuardRewriteSuccess`
3. 当前最终代码配置：
   `Startup = 0.00s`
   `Window = 0.45s`
   `Recovery = 0.10s`

## 验收矩阵

### 1. 空闲态进入 Guard

- 状态：已覆盖
- 结论：通过
- 证据：
  `CombatInputEval input=Guard result=ExecuteNow`
  `GuardActivate`
  `GuardWindowOpened`
- 说明：
  日志显示在无活跃动作时，Guard 输入会立即执行，并进入 Active 窗口。

### 2. 正确窗口内成功格挡

- 状态：已覆盖
- 结论：通过
- 证据：
  `AttackContact -> HitConfirmed -> GuardRewrite -> GuardRewriteSuccess`
  `PlayerHitResult result=GuardRewritten hit=false`
- 说明：
  Guard 成功时，原本会落到玩家身上的命中结果已被改写，且最终结果与普通受击可区分。

### 3. 过早输入

- 状态：已覆盖
- 结论：通过
- 证据：
  `HostileProbe_1` 与 `HostileProbe_6` 中，`GuardWindowClosed` 先于 `AttackContact`；
  最终结果停在 `HitConfirmed`；
  没有出现对应攻击的 `GuardRewriteSuccess`。
- 说明：
  这说明玩家提前按下 Guard 时，窗口会先结束，之后仍会正常吃到敌对命中。

### 4. 过晚输入

- 状态：已覆盖
- 结论：通过
- 证据：
  `HostileProbe_2` 中，`AttackContact -> HitConfirmed` 先落地；
  随后才出现 `GuardActivate`；
  最终结果保持为 `HitConfirmed`。
- 说明：
  这说明命中已经落地后再按 Guard，不会回头改写已经完成的受击结果。

### 5. 不输入直接吃招

- 状态：已覆盖
- 结论：通过
- 证据：
  `HostileProbe_1` 中存在完整链路：
  `AttackStarted -> HitWindowOpened -> AttackContact -> HitConfirmed`；
  同一攻击期间没有任何 `GuardActivate`。
- 说明：
  这说明在完全不输入 Guard 的情况下，玩家会按预期直接进入普通受击结果。

### 6. 连续重复测试的一致性

- 状态：已覆盖
- 结论：通过
- 证据：
  `HostileProbe_1` 到 `HostileProbe_10` 均出现 `GuardRewriteSuccess`
- 说明：
  这说明 Guard 改写链路在当前调试条件下具备稳定可重复性。

### 7. 普通攻击 Recovery 与 Guard 的边界

- 状态：已覆盖
- 结论：通过
- 证据：
  前序联调已确认 Guard 可在普通攻击 `Recovery / LogicEnded` 打断进入。
- 说明：
  这是当前 PRD 要求覆盖的关键边界之一，已完成验证。

### 8. 普通攻击 Startup / Active 与 Guard 的边界

- 状态：已覆盖
- 结论：通过
- 证据：
  当前公共动作上下文与 Guard Ability 均已收紧；
  Guard 不再允许在普通攻击 `Startup / Active` 越权打断。
- 说明：
  这部分结论来自本轮代码与前序跑测的联合验证。

### 9. Dodge 与 Guard 的边界

- 状态：已覆盖到当前最小规则
- 结论：通过
- 证据：
  当前代码只允许 Guard 与“当前基础 Dodge”发生关系，未再向其他动作扩散。
- 说明：
  本轮目标是验证当前最小白名单，不扩写到更完整动作系统。

## 可读反馈验收结论

1. 当前最小可读反馈已基本成立。
2. 测试和程序已能通过 HUD 与结构化日志区分：
   攻击开始；
   Guard 窗口开启；
   普通命中；
   Guard 成功改写；
   Guard 生命周期结束。
3. 当前仍缺的是更直观的正式动作表现，而不是“完全看不见结果”。

## 当前问题归因

1. 逻辑：
   当前基础 Guard 主链路基本达标，未发现需要回退前置任务的阻断性逻辑问题。
2. 配置：
   Guard 时间窗仍处于调参阶段；本轮曾临时使用 `1.00s` 窗口拿稳定成功样本，当前最终代码已收回到 `0.45s`。
3. 资源：
   当前没有正式 Guard 动作/蒙太奇资源，导致体感判断严重依赖 HUD 与日志。
4. 表现：
   这是当前最大短板。没有正式动作时，玩家在 PIE 里很难仅靠肉眼判断“这次到底是成功挡住了，还是只是命中时机碰巧过去了”。

## 本轮结论总览

1. 本 task 的核心目标“让 Guard 变得可观察、可判断、可说明”已完成。
2. 已完成的部分主要集中在：
   最小反馈口径；
   成功样本白盒证据；
   失败样本白盒证据；
   边界规则收紧；
   连续一致性证明。
3. 当前仍存在的主要短板已经不在“格挡是否成立”，而在“缺少正式动作资源时的肉眼可读性较弱”。
4. 因此，本 task 当前可判定为：
   主逻辑达标；
   最小反馈达标；
   白盒验收达标；
   可按测试视角归档收尾。

## Spec 判断

1. 本轮没有新增需要写回 `.trellis/spec/` 的稳定项目级规则。
2. 当前结论属于阶段性 Guard 验收结果，保留在 task 文档比上升到项目规范更合适。
