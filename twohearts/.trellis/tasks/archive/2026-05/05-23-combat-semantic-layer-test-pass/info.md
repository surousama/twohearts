# 公共战斗语义层白盒回归结论

## 本轮范围

* 白盒审查对象：
  `05-23-combat-action-context-foundation`
  `05-23-normal-attack-semantic-bridge`
  `05-23-dodge-semantic-bridge-and-interrupt-unification`
  `05-23-combat-input-evaluation-preinput-hook`
* 本轮方法：
  代码 review + 调试链路检查
* 本轮未做：
  Unreal Editor PIE 实跑
  新一轮本地构建

## 本轮修复进展

* 已完成代码修复：
  `UTwoHeartsGA_Dodge::ActivateAbility` 改为先做无副作用启动校验与中断许可校验，再 `CommitAbility`，最后才执行真实普攻打断；
  `AtwoheartsCharacter::HandleBufferedCombatInput` 在 `ForwardToActiveAbility` 未命中 active ability 时改为明确失败，不再伪装成成功缓冲；
  `AtwoheartsCharacter::HandleAbilityInputPressed` 的无上下文回退路径已补全 `ActivateMatchingAbility` 路由
* 已完成本地验证：
  `G:\\UE_5.6\\Engine\\Build\\BatchFiles\\Build.bat twoheartsEditor Win64 Development -Project=g:\\twohearts\\twohearts\\twohearts.uproject -WaitMutex -FromMsBuild`
  构建通过
* 待你继续用 PIE 复测确认：
  问题 1 / 2 / 3 当前状态可先视为“代码已修复，待运行态回归”

## 问题记录

### 1. Dodge 在自身启动失败时，仍可能先打断普攻

* 状态：已确认代码路径缺陷
* 优先级：高
* 触发条件：
  当前普攻处于允许被 Dodge 打断的 `Recovery / LogicEnded`；
  Dodge 输入进入后，后续任一启动步骤失败，例如方向解析失败、`CommitAbility` 失败、Montage / Root Motion 校验失败、Montage Task 创建失败
* 复现步骤：
  1. 让角色进入普攻可被闪避打断阶段
  2. 按下 Dodge
  3. 让 Dodge 后续启动链路失败
* 现象：
  普攻会先被中断，但 Dodge 自身没有成功执行
* 预期：
  只有当 Dodge 已确认可正常启动时，才允许消耗这次打断机会并中断普攻
* 实际：
  `UTwoHeartsGA_Dodge::ActivateAbility` 先执行 `TryInterruptCurrentActionByDodge()`，之后才做 `ResolveDodgeDirection`、`CommitAbility` 与 `StartDodgeExecution`
* 影响：
  玩家可能看到“攻击被吃掉，但闪避也没出来”的硬卡顿体验；
  这是会直接影响手感与稳定性的主链路问题
* 代码落点：
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:48)
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:54)
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:61)
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:68)
* 问题说明：
  当前统一打断入口的调用时序过早，导致“打断成功”和“Dodge 真正成功启动”没有绑定成同一事务
* 建议修法：
  先完成 Dodge 可启动性校验与 `CommitAbility`，确认后再执行对当前动作的真实打断；
  若必须提前判定，也应具备失败回滚策略
* 复测重点：
  资源缺失、Root Motion 关闭、冷却 / Tag 阻塞、Montage Task 创建失败等异常路径下，普攻不应被白白打断

### 2. `BufferInput -> ForwardToActiveAbility` 在未真正转发成功时仍返回成功

* 状态：已确认代码路径缺陷
* 优先级：中
* 触发条件：
  输入评估结果为 `BufferInput` 且消费路径为 `ForwardToActiveAbility`，但运行时没有找到对应的 active ability 实例
* 复现步骤：
  1. 让输入评估给出 `ForwardToActiveAbility`
  2. 同时制造公共动作上下文与活跃 Ability 实例不同步，或切段边缘 active instance 已失效
* 现象：
  输入会被当成“已缓冲/已处理”记录，但实际上没有任何 Ability 消费它
* 预期：
  若没有真正转发到 active ability，应记录失败或至少区分为“未转发成功”
* 实际：
  `HandleBufferedCombatInput` 在未转发成功时仍返回 `true`，并改口记录为“留给后续消费者”，但当前实现并没有真正的后续消费者
* 影响：
  会造成输入被悄悄吞掉；
  HUD / 日志还会给出“输入已接受”的假象，增加联调误判成本
* 代码落点：
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:732)
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:737)
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:756)
* 问题说明：
  `InputEvaluation` 说的是“转发给 active ability”，但处理实现允许它在未转发成功时静默降级为“预留给未来消费者”
* 建议修法：
  当 `ConsumptionRoute == ForwardToActiveAbility` 且实际未找到 active ability 时，应明确记录失败并返回 `false`，不要伪装成成功缓冲
* 复测重点：
  普攻切段边缘、取消边缘、动作上下文切换边缘下连续按攻击，确认 `BufferInput / ForwardToActiveAbility` 不会出现“日志说成功、实际没反应”

### 3. 无动作上下文组件时的回退路径自相矛盾

* 状态：已确认代码路径缺陷
* 优先级：中
* 触发条件：
  `CombatActionContextComponent` 缺失或未来被替换 / 失效
* 复现步骤：
  1. 在缺失 `CombatActionContextComponent` 的前提下触发攻击或闪避输入
* 现象：
  代码先把结果设成 `ExecuteNow`，随后又因为缺少 `ActivateMatchingAbility` 路由而直接失败
* 预期：
  回退路径应真的走“立即执行”，而不是自相矛盾地报错退出
* 实际：
  `HandleAbilityInputPressed` 只设置了 `Result=ExecuteNow`，没有补 `ConsumptionRoute=ActivateMatchingAbility`
* 影响：
  当前默认构造路径下不易触发，但属于后续重构/异常场景下的潜在回归点
* 代码落点：
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:312)
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:318)
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:340)
* 问题说明：
  这是一个 latent bug，当前不是主路径问题，但逻辑上已经不成立
* 建议修法：
  回退分支补全 `ConsumptionRoute=ActivateMatchingAbility`，或统一改为明确 reject 并给出原因
* 复测重点：
  若后续组件初始化、挂载方式或调试替身发生变化，需要回归这条保底路径

## 风险与观察盲区

### 1. 普攻切段时公共动作上下文存在短暂空窗

* 状态：需继续观察
* 风险说明：
  普攻上一段 `FinishAction` 后，下一段才重新 `BeginAction`；
  公共动作上下文不会把 `1 -> 2 -> 3` 连段视为一个连续动作流，而是段与段之间存在短暂 inactive 空档
* 影响：
  在切段边缘抓输入时，可能得到 `No active combat action is currently registered`；
  需要确认这是否会引发手感问题或仅是日志层面的瞬时状态
* 代码落点：
  [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:303)
  [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:321)
  [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:133)

### 2. Dodge 的 `LogicEnded` 缺少独立、稳定的可观察信号

* 状态：需继续观察
* 风险说明：
  代码里有 `HandleDodgeFinished()` 和 `DodgeLogicEnded` 事件，但实际 Notify 处理路径直接走 `FinishDodge(false)`；
  当前更像是“结束时顺带标记 LogicEnded”，而不是暴露出一个稳定可观察的“已可衔接、但尚未完全结束”窗口
* 影响：
  难以仅靠 HUD/最后一条 Dodge 事件判断 `Recovery -> LogicEnded -> FinishAction` 是否真的分离存在
* 代码落点：
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:620)
  [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:626)
  [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:590)
  [twoheartsDebugHUD.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsDebugHUD.cpp:210)

### 3. HUD 只能看当前快照，顺序核对仍依赖结构化日志

* 状态：需继续观察
* 风险说明：
  HUD 的公共动作上下文只展示当前快照，不保留 `BeginAction -> TransitionPhase -> LogicEnded -> FinishAction` 历史；
  只看面板不足以完成严格白盒时序验证
* 影响：
  PIE 跑测若不同时开日志，容易把“观察信号不够”误判为“功能错了”
* 代码落点：
  [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:23)
  [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:47)
  [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:68)
  [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:88)
  [twoheartsDebugHUD.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsDebugHUD.cpp:121)

## PIE 跑测策划清单

### 公共动作上下文

* 普攻单段完整播放，核对日志顺序是否稳定为：
  `BeginAction -> TransitionPhase(Active/Recovery) -> LogicEnded -> FinishAction`
* Dodge 单次完整播放，核对是否能明确分辨：
  `Startup / Active / Recovery / LogicEnded / FinishAction`
* 对照 HUD 的 “Public Action Context” 与结构化日志，确认 `ActionType / Phase / LogicEnded / EndReason / LastReason` 一致

### 普攻 1-2-3 连段

* 原地普攻 `1 -> 2 -> 3`
* 移动中普攻 `1 -> 2 -> 3`
* 在 `Startup / Active / Recovery / LogicEnded` 各阶段再次按攻击键，分别记录：
  `CombatInputEval.Result`
  `CombatInputEval.Route`
  实际是否成功排队 / 执行 / 无反应
* 在 `1 -> 2`、`2 -> 3` 切段边缘连续按攻击，观察是否出现“上下文显示无 active action”的瞬时空窗

### 普攻被 Dodge 打断

* 在普攻 `Startup` 按 Dodge，应被拒绝
* 在普攻 `Active` 按 Dodge，应被拒绝
* 在普攻 `Recovery` 按 Dodge，应允许打断
* 在普攻 `LogicEnded` 按 Dodge，应允许打断
* 重点检查：
  被允许打断时，普攻是否正确结束；
  被拒绝时，普攻是否保持原动作不乱相；
  Dodge 是否真的成功启动，而不是“普攻没了但 Dodge 也没出”

### Dodge 自身行为

* 原地 Dodge 与八方向 Dodge，核对方向解析与 Montage 选择
* 检查 Root Motion 生效、无敌窗口开始/结束、冷却开始/结束
* Dodge 完成后立刻接普攻，确认是否存在可衔接窗口
* 若能构造异常资源路径，重点验证：
  Dodge 启动失败时，不应提前吃掉普攻打断机会

### 输入评估三态

* 记录 `ExecuteNow / BufferInput / Reject` 三态是否与实际行为一致
* 对 `BufferInput` 重点区分：
  是真的转发给 active ability
  还是只是被记录为“留给未来消费者”
* 对 `Reject` 重点验证：
  玩家是否能从 HUD / 日志清楚看到被拒原因，而不是无反馈卡死

## 当前结论

* 可以确认：首轮公共动作语义层已经形成可审查的正式承载结构
* 当前主要阻塞不在“有没有公共语义层”，而在：
  Dodge 打断时序存在主链路缺陷；
  输入缓冲成功语义与真实消费结果存在偏差；
  运行时观察信号仍不足以完全替代日志级白盒验证
* 在修复高优先级问题前，不建议直接判定该语义层“可稳定收口”
