# 普攻连段连续性优化白盒问题记录

## 本轮结论

1. 当前 task 的核心验收被阻断。
2. 已确认的主症状有两个：
   - 无 `CombatPhase_AdvanceNextSegment` notify 配置时，普通攻击只能打出第一段，进不到第二段。
   - 普攻期间闪避失效，按空格无法打断并进入闪避。
3. 本轮依据：
   - 用户已在 PIE 中复现上述问题。
   - 我已对白盒代码路径完成 review，定位到普攻阶段推进、输入裁决和闪避打断链路上的高风险落点。
4. 本轮未在当前终端自行启动 Unreal Editor 复跑，因此“现象是否仍 100% 稳定复现”以用户 PIE 结果为准；下述根因说明分为“高置信代码缺陷”和“需程序侧补日志确认”两类。

---

## 问题 1：普攻提前切段路径可能把正常续段误判为中断，导致无 notify 时连不到第二段

1. 问题标题
   普攻续段提前切出链路存在自取消风险，`1 -> 2` 可能被拦死
2. 优先级
   `P1`
3. 状态
   `已复现`
4. 触发条件
   第一段普攻期间输入下一段，系统走“提前切到下一段”逻辑；无 `CombatPhase_AdvanceNextSegment` notify 时，主要依赖 fallback 路径触发。
5. 复现步骤
   1. 进入 PIE。
   2. 角色保持当前无 `CombatPhase_AdvanceNextSegment` notify 的配置。
   3. 打出第一段普通攻击，并在允许连段的时机继续按普攻。
   4. 观察是否进入第二段。
6. 现象
   第一段结束后无法稳定进入第二段，表现为“只能打一段”。
7. 预期
   即使暂未配置 `CombatPhase_AdvanceNextSegment` notify，fallback 也应至少保证普攻可以继续正常续段；最差也应退回旧的段尾衔接，而不是直接断链。
8. 实际
   当前逻辑会在“提前切段”路径上主动 `Montage_Stop`，同时仍保留 `OnInterrupted / OnCancelled` 回调；这条链路存在把“正常提前续段”记成“被打断”的风险，进而使下一段激活流程不成立。
9. 影响
   - 直接阻断当前 task 的主验收项。
   - `1 -> 2 -> 3` 连段不再可靠，PRD 的“无 notify 也能靠 fallback 工作”结论当前不能成立。
10. 代码落点
   - [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:463)
   - [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:505)
   - [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:205)
   - [TwoHeartsGA_NormalAttackBase.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp:312)
11. 问题说明
   - `TryAdvanceToNextSegment(...)` 在确认可续段后，会先执行 `Montage_Stop(0.05f, Montage)`，然后手动 `FinishSegment(false)`。
   - 同一个 Ability 仍然绑定着 `HandleMontageInterrupted()` / `HandleMontageCancelled()`，而这两个回调都会调用 `FinishSegment(true)`。
   - 这意味着“主动提前切段”与“蒙太奇被打断”共用了一套回调收尾，存在竞争关系：一旦 `Montage_Stop` 先触发中断回调，就可能把本应正常续段的收尾记成 `bWasCancelled=true`，从而让后续 `bShouldActivateNext` 不成立。
   - 这条缺陷与用户看到的“无 notify 时只能打一段”高度一致，因为无 notify 配置下，fallback 正是当前提前切段的主要入口。
12. 建议修法
   - 程序侧应把“主动提前续段”与“异常中断”分流，不要共用同一个 `Montage_Stop -> Interrupted -> FinishSegment(true)` 语义。
   - 可选方向：
     - 在主动续段前设置显式标记，使 `HandleMontageInterrupted()` 忽略这次中断；
     - 或在主动续段前先解绑相关 montage 回调，再以单一入口完成 `FinishSegment(false)`；
     - 或改成不依赖 `Montage_Stop` 的切段方式，避免把正常续段包装成“被打断”。
13. 复测重点
   - 无 notify 配置下，`1 -> 2`、`2 -> 3` 是否恢复。
   - 有 notify 配置下，提前切段是否也稳定。
   - 晚按、压线按、单按不续段三类输入是否都符合 PRD。

---

## 问题 2：闪避是否可打断普攻，当前被强耦合到阶段推进；无 notify / fallback 不成立时会整体失效

1. 问题标题
   闪避打断完全依赖普攻阶段推进，当前回退链路失效时空格会被整体拒绝
2. 优先级
   `P1`
3. 状态
   `已复现`
4. 触发条件
   普攻进行中按空格，尤其是在当前 montage 没有完整 notify 配置、需要依赖 fallback 推进阶段时。
5. 复现步骤
   1. 进入 PIE。
   2. 先打出第一段普通攻击。
   3. 在攻击过程中多次按空格，覆盖偏早、偏中、偏晚三个时机。
   4. 观察角色是否能进入闪避。
6. 现象
   角色无法通过空格从当前普攻中打断并进入闪避。
7. 预期
   至少在普攻进入 `Recovery` 或 `LogicEnded` 后，闪避应可通过公共动作上下文打断当前普攻。
8. 实际
   当前闪避的放行完全依赖 `CanCurrentActionBeInterruptedBy(Dodge)`；只要当前普攻阶段没有被正确推进到 `Recovery / LogicEnded`，空格就会持续被拒绝。
9. 影响
   - 闪避出现明显回归，影响第二章基础战斗当前顺序中的“基础闪避正式落地收口”。
   - 这不是单一手感问题，而是动作可打断关系被普攻新逻辑连带破坏。
10. 代码落点
   - [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:100)
   - [TwoHeartsCombatActionContextComponent.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp:119)
   - [TwoHeartsGA_Dodge.cpp](/g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:197)
   - [twoheartsCharacter.cpp](/g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:329)
11. 问题说明
   - `EvaluateInputForAction(...)` 会先读取当前公共动作上下文。
   - 对闪避而言，真正能否立即执行，最终取决于 `CanCurrentActionBeInterruptedBy(Dodge)`。
   - 该函数当前只在“当前动作是普攻，且阶段处于 `Recovery` 或 `LogicEnded`”时返回 `true`。
   - 因此，只要普攻阶段推进没有按预期进入这两个阶段，`Dodge` 输入就没有任何兜底路径，会直接在角色输入入口被判成 `Reject`。
   - 结合用户当前复现条件，“无 notify 时第二段也起不来”说明这轮改动至少已经让 fallback 相关阶段推进不可信；在这种前提下，闪避整体失效是同一条状态链路的下游症状。
12. 建议修法
   - 先验证无 notify 路径下 `Startup -> Active -> Recovery -> LogicEnded` 四个阶段是否真的按 fallback 推进，而不是只在设计上假定 fallback 一定工作。
   - 程序侧建议补以下观察信号：
     - section length 是否成功取到；
     - `SchedulePhaseFallbacks(...)` 是否实际挂上了定时器；
     - 普攻运行时是否真的进入了 `Recovery`。
   - 在这些观测确认前，不建议把“闪避打断失败”简单归结为资源侧 notify 没配，因为当前 C++ 链路本身没有提供足够强的兜底证明。
13. 复测重点
   - 无 notify 配置下，普攻阶段是否仍可推进到 `Recovery / LogicEnded`。
   - 进入 `Recovery` 后空格是否必定能打断。
   - 提前按空格应被拒绝，窗口内按空格应放行，窗口后按空格应按最终设计处理。

---

## 建议上游程序优先确认的观察点

1. `TryAdvanceToNextSegment(...)` 主动 `Montage_Stop` 后，是否先收到了 `HandleMontageInterrupted()`。
2. `GetNormalAttackSectionLength(...)` 在当前角色资源配置下是否稳定大于 `0.0f`。
3. 无 notify 配置时，`SchedulePhaseFallbacks(...)` 是否确实触发了 `Recovery` 和 `NextSegmentAdvance` 两个 fallback。
4. 当前 HUD / 日志里是否能直接看到：
   - 普攻阶段变化；
   - `AdvanceWindowOpened`；
   - `AdvanceSegmentReady`；
   - `InputRejected` 的具体 reason；
   - `DodgeRejected` 的具体 phase。

## 当前回归结论

1. 当前提交 `58b9459` 不应视为通过该 task 的白盒验收。
2. 当前更像“连段提前切换逻辑引入了阶段与收尾链路回归”，而不是单纯资源没配完。

---

## 问题 3：即使已配置 `CombatPhase_AdvanceNextSegment` notify，普攻 1 仍明显播过了标记点之后才切段

1. 问题标题
   普攻 1 的提前切段时机晚于 notify 摆放点，当前切段触发不符合资源联调预期
2. 优先级
   `P1`
3. 状态
   `用户 PIE 已复现`
4. 触发条件
   已在普通攻击 Montage 中配置 `CombatPhase_AdvanceNextSegment` notify，并在普攻 1 可续段窗口内继续按普攻。
5. 用户体感现象
   普攻 1 明显已经播到了“起身动作”之后才切到下一段；而当前 notify 摆放点在起身动作之前。
6. 预期
   只要下一段输入已成立，普攻 1 应在 notify 对应的切段点附近完成衔接，而不是继续明显播放 notify 后的收尾动作。
7. 初步判断
   - 当前问题已不再只是“无 notify 时 fallback 是否工作”，而是“有 notify 时是否真的以 notify 为主导切段”。
   - 若 notify 已被正确触发，但视觉上仍明显晚于标记点，程序侧需要重点核查：
     - `OpenNextSegmentAdvanceWindow(...)` 是否虽然开窗，但没有立即完成切段；
     - `Montage_Stop(0.05f)` 的实际停止方式是否让当前 Section 继续残留了过多尾帧；
     - `FinishSegment(false)` 之后到下一段 Ability 真正起播之间，是否仍有额外延迟。
8. 当前影响
   - 即使“能连上”，也还没有达到 PRD 要求的“在主要攻击表达完成后尽快衔接”的验收口径。
   - 该问题优先级高于纯资源微调，因为用户已经明确指出 notify 摆放点早于当前实际切段观感。

---

## 问题 4：普攻 2 / 普攻 3 视觉表现疑似串段，用户肉眼观察两者看起来完全一样

1. 问题标题
   普攻 2 与普攻 3 的实际播放结果疑似串段或误激活，当前视觉上无法稳定区分
2. 优先级
   `P1`
3. 状态
   `用户 PIE 已复现`
4. 触发条件
   连续执行 `1 -> 2 -> 3` 普攻连段时，重点观察第二段与第三段动作表现。
5. 用户体感现象
   - 普攻 2 目前一直有些奇怪的问题。
   - 普攻 2 设计上应是“上扬挥砍起手”。
   - 但用户肉眼观察，普攻 2 和普攻 3 的动画看起来完全一样。
6. 预期
   普攻 2 与普攻 3 应分别稳定播放各自对应的 Section，不应出现视觉上稳定重合、串段或错段。
7. 当前与日志的对照
   - 本轮日志中出现过：
     - `Segment 2` 正常进入 `LogicEnded / SegmentFinished / FinishAction Completed`
     - 随后又出现 `Segment 1 was interrupted before completion`
     - 再随后出现 `Segment 3 was interrupted before completion`
   - 这说明当前不仅有“切段时机”问题，还存在“段实例激活/取消顺序异常”的高风险信号。
8. 初步判断
   程序侧需要重点核查：
   - deferred activation 触发后，是否可能又被新的普通攻击输入立即重新拉起 `Segment1`；
   - `TryExecuteCombatInputNow(...)` 与当前 active action 结束时机之间，是否存在把后续输入重新判成“从起手段重新开打”的窗口；
   - `Segment2 -> Segment3` 的激活是否成功，但视觉上被随后错误激活/取消的别的段覆盖。
9. 当前影响
   - 该问题会直接破坏 `1 -> 2 -> 3` 顺序稳定性，是当前 task 的核心阻断项之一。
   - 在未排清前，不能把“普攻 2 看起来像普攻 3”简单归因于资源摆放或 notify 位置。

---

## 本轮新增日志观察（基于已配置 notify 的一次跑测）

1. 已确认修复生效的局部现象
   - 日志出现：
     `Ignored montage interrupted callback while segment 2 was intentionally stopping`
   - 随后 `Segment 2` 仍正常进入：
     `LogicEnded`
     `SegmentFinished`
     `FinishAction ... reason=NormalAttackEnded`
   - 这说明“主动提前切段被误记成 Interrupted/Cancelled”这条链路，至少在 `2 -> 3` 的一次样本上已被部分修正。
2. 仍然异常的现象
   - 同一轮日志里又出现：
     `Segment 1 was interrupted before completion`
     `Segment 3 was interrupted before completion`
   - 且收尾理由是：
     `reason=NormalAttackCancelled`
   - 不是 `InterruptedByDodge`。
3. 当前综合判断
   - 已修掉一个核心缺陷，但当前实现仍未稳定通过验收。
   - 剩余问题已经从“单点切段误判”扩展为：
     - 切段时机仍偏晚；
     - 段实例激活/取消顺序可能异常；
     - `1 / 2 / 3` 的真实播放段次序可能存在串段或错段。
