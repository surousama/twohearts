# 基础格挡白盒 review（2026-05-27）

## 本轮范围

1. 以资深测试工程师视角，对 `05-25-basic-guard-implementation` 已提交代码做静态白盒 review。
2. 本轮未进行 PIE 实机跑测；结论以代码路径、调试入口和 Trellis 任务配置检查为主。

## 结论总览

1. 当前 Guard 主链路已经正式接入 `UE5 + GAS + C++`，没有回退到 Character 私有状态机，这是本轮的正向结论。
2. 代码里存在 1 个阻断当前验收口径的问题、1 个中风险规则漏洞，以及 1 组高概率缺失的编辑器配置项。
3. `implement.jsonl` / `check.jsonl` 原先仍是种子行，已补成真实上下文条目，后续继续实施或复核时不再缺少 Trellis 配置。

## 问题记录

### 1. Guard 改写后仍保留 `bHitConfirmed=true`

- 优先级：高
- 状态：已复现（代码层）
- 触发条件：Guard 在活动窗口内把 `HitConfirmed` 改写为 `GuardRewritten`
- 现象：结果类型被改成 `GuardRewritten`，但 `bHitConfirmed` 没有同步改写
- 预期：格挡成功后，下游应能明确区分“命中玩家”和“被 Guard 改写”
- 实际：`RewriteLastPlayerHitResultForGuard` 只改了 `ResultType`、`bCanBeRewrittenByGuard`、`Detail`、`SourceSignalType`、`ResultTimestampSeconds`，没有改 `bHitConfirmed`
- 影响：
  1. 调试 HUD 仍会把这次结果显示为命中成功
  2. 后续若有逻辑只看 `bHitConfirmed`，会把 Guard 成功误判成受击成功
  3. 与 PRD 中“把原本命中玩家的结果改写为格挡成功”的验收语义不完全一致
- 代码落点：
  1. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp:132`
  2. `Source/twohearts/twoheartsDebugHUD.cpp:328`
- 建议修法：
  1. 明确 `GuardRewritten` 的布尔伴随语义
  2. 若 `bHitConfirmed` 表示“玩家真的吃到命中”，则 Guard 改写时应改成 `false`
  3. 若 `bHitConfirmed` 只表示“攻击接触曾发生”，则应补一个新的最终结果布尔或 HUD 展示逻辑，避免把 Guard 成功显示成受击成功
- 复测重点：
  1. HUD 中 `LastHitResult` 的 `hit` 字段
  2. `GuardRewriteSuccess` 日志出现后，最终结果是否仍被任何下游视为普通命中

### 2. Guard 的打断规则对“非普攻/非闪避动作”过于宽松

- 优先级：中
- 状态：已复现（代码层）
- 触发条件：当前活跃动作不是 `NormalAttack` / `Dodge`，但玩家输入 Guard
- 现象：输入评估层与 Ability 启动前检查都默认放行
- 预期：Guard 是否能打断其他动作，应显式由公共动作语义层给出规则，而不是默认全部放行
- 实际：
  1. `EvaluateInputForAction` 直接把 Guard 评估为 `ExecuteNow`
  2. `CanInterruptCurrentActionByGuard` 对除 `NormalAttack` / `Dodge` 之外的动作直接 `return true`
- 影响：
  1. 当前虽然只覆盖最小链路，但未来新增动作后，可能被 Guard 在未定义阶段直接打断
  2. 这会削弱公共动作上下文“显式定义打断关系”的约束
- 代码落点：
  1. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:165`
  2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.cpp:173`
- 建议修法：
  1. 当前阶段若只允许 Guard 打断普攻 Recovery 和基础 Dodge，就把规则收窄到显式白名单
  2. 至少把“其他动作默认允许”改成“其他动作默认拒绝，待后续明确规则再开”
- 复测重点：
  1. 后续新增动作接入后，Guard 是否仍会越权打断
  2. 公共动作上下文与各 Ability 的打断判断是否一致

### 3. 编辑器侧高概率仍缺 Guard 输入与默认 Ability 配置

- 优先级：高
- 状态：需继续观察（高概率配置缺口）
- 触发条件：使用已有 `BP_ThirdPersonCharacter` 直接进 PIE
- 现象：即使 C++ 已提交 Guard 代码，运行时仍可能没有 Guard 输入或没有被授予 Guard Ability
- 预期：已有角色蓝图和输入映射应显式补齐 Guard 相关配置
- 实际：
  1. `GuardAction` 只有在编辑器里被赋值后才会绑定输入
  2. `DefaultCombatAbilities` 只有在数组为空时，构造函数才会自动塞入 Guard；已有蓝图若已经序列化过旧数组，不会自动补这一项
- 影响：
  1. 代码层“已完成”但 PIE 里按键无响应
  2. 会把配置缺口误判成逻辑 bug，拖慢联调
- 代码落点：
  1. `Source/twohearts/twoheartsCharacter.h:251`
  2. `Source/twohearts/twoheartsCharacter.h:255`
  3. `Source/twohearts/twoheartsCharacter.cpp:162`
  4. `Source/twohearts/twoheartsCharacter.cpp:242`
- 建议修法：
  1. 打开 `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`
  2. 在角色默认值里确认 `GuardAction` 已指向对应 `InputAction`
  3. 在 `DefaultCombatAbilities` 数组里确认存在 `UTwoHeartsGA_Guard + InputID=Guard`
  4. 在玩家控制器使用的 `InputMappingContext` 中确认已经把 Guard 对应按键映射到该 `InputAction`
- 复测重点：
  1. PIE 中按下 Guard 后是否出现 `GuardActivate` / `GuardRejected` 日志
  2. HUD 是否能观察到 `Guard Active / Window / Phase`

## 本轮测试结论

1. 架构边界：基本达标。Guard 主体仍在 Ability、公共动作上下文和受击结果入口内，没有回退到 Character 私有状态机。
2. 逻辑稳定性：未完全达标。`GuardRewritten` 的伴随状态定义还不闭合。
3. 配置完整性：未达标。当前任务原始 JSONL 上下文缺失，编辑器资产侧也高概率仍需手工补齐 Guard 配置。
4. 跑测结论：本轮未做 PIE，因此“体感是否正确、窗口是否合手”仍待 `05-25-guard-feedback-and-whitebox-validation` 补完。
