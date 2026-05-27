# 普通攻击三段连续性优化 - 配置与联调说明

## 当前结论

1. 本轮程序侧已经把普通攻击下一段的起播时机，从“当前段 `FinishSegment` 之后再激活”前移为“到达可切出点后即可提前衔接下一段”。
2. 这个“可切出点”现在支持两套来源：
   `CombatPhase_AdvanceNextSegment` Montage Notify；
   `NextSegmentAdvanceFallbackNormalizedTime` 归一化时间兜底。
3. 因此当前逻辑已经可运行，但若想把 `1 -> 2`、`2 -> 3` 的手感收得更准，仍推荐在动画资源里显式配置 `CombatPhase_AdvanceNextSegment`。

## 本轮代码落点

1. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   新增：
   `NextSegmentAdvanceNotifyName`
   `NextSegmentAdvanceFallbackNormalizedTime`
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
   新增：
   `OpenNextSegmentAdvanceWindow`
   `TryAdvanceToNextSegment`
   `HandleFallbackOpenNextSegmentAdvanceWindow`
3. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
   普攻在 `Recovery` 且尚未 `LogicEnded` 时，跟进输入仍可继续转发给当前 Ability，不再一律拖到段尾缓冲消费。

## 需要在 Unreal Editor 里做的配置

1. 打开普通攻击使用的 Montage 资源。
   当前项目里通常是：
   `AM_Melee_NormalAttackCombo`
2. 检查三段普通攻击对应的 Section。
   确认 `1 / 2 / 3` 三段仍然分别对应角色配置里的 `NormalAttackSectionNames`。
3. 在第 1 段和第 2 段里增加同名 Notify：
   `CombatPhase_AdvanceNextSegment`
4. Notify 的摆放位置建议放在：
   “这一段主要攻击表达已经完成，但还没进入完整硬收尾”的位置。
5. 第 3 段通常不需要再放这个 Notify。
   因为它没有后续 `NextSegmentAbilityTag`，程序不会继续推进下一段。

## Notify 摆放口径

1. 不要把 `CombatPhase_AdvanceNextSegment` 放得早于主要命中表达。
   否则连段会显得抢切，前一段攻击还没读完就被吃掉。
2. 不要把它放到接近 Section 末尾。
   否则虽然逻辑正确，但体感上仍然接近旧的“段尾接段”。
3. 一个实用判断标准是：
   如果玩家已经在窗口内按出下一段，那么当前段在这个点之后就可以被顺滑截断；
   如果玩家没按，则这一段继续自然收尾。
4. 如果三段共用同一 Montage，只需要 Notify 名字统一为：
   `CombatPhase_AdvanceNextSegment`
   真正的差异靠每个 Section 内的摆放位置体现。

## 当前程序兜底时间

1. 当前 fallback 不是“从 Finish 点往前推”，而是“从 Section 起点按长度比例定时”。
2. 当前默认值是：
   `ActivePhaseFallbackNormalizedTime = 0.20`
   `RecoveryPhaseFallbackNormalizedTime = 0.60`
   `NextSegmentAdvanceFallbackNormalizedTime = 0.70`
   `LogicEndedFallbackNormalizedTime = 0.85`
3. 这表示如果某段 Section 长度是 `1.0s`，默认会在 `0.70s` 打开“可提前切下一段”的兜底窗口。
4. 即使还没配 Notify，当前逻辑也会按这个 fallback 工作；
   但这只是兜底，不建议长期只靠它做手感收口。

## 联调时建议重点观察

1. 连按普通攻击时：
   `1 -> 2`
   `2 -> 3`
   是否已经不再表现为“上一段完整硬收尾后，下一段才开始”。
2. 单按一段时：
   没有后续输入的情况下，当前段是否仍能自然收尾，没有异常抢切。
3. 接近窗口末尾时补按：
   是否仍能在允许窗口内顺利接上下一段。
4. 明显过晚输入时：
   是否保持“不接下一段”的明确结果，而不是无限放宽窗口。

## 给资源联调的明确建议

1. 先加 `CombatPhase_AdvanceNextSegment`，不要先删 fallback。
2. 先把第 1 段和第 2 段各配一个大致合理的位置，再进 PIE 看体感。
3. 如果仍有停顿，再优先微调 Notify 位置，而不是先改程序逻辑。
4. 只有当 Notify 位置已经合理，但停顿仍明显存在时，才考虑剩余问题是否主要来自动画资产尾巴本身过长。
## 2026-05-26 Notify 探针总结

1. 本轮新增了更强的普攻运行时日志，覆盖：
   `PlaySegment`
   `MontageNotifyBegin`
   `MontageNotify`
   `AdvanceWindowOpened`
   `AdvanceSegmentReady`
   `AdvanceSegmentAttempt`
   并新增文件日志：
   `Saved/CombatDebug/normal-attack-debug.log`
2. 文件日志已经确认，运行时播放的确实是本轮编辑过的 Montage 资源：
   `/Game/Chinese_Warrior/Animations/Combat_Montage/AM_Melee_NormalAttackCombo.AM_Melee_NormalAttackCombo`
3. 但这轮探针跑测里，没有收到任何：
   `MontageNotifyBegin`
   `MontageNotify`
4. 这说明当前普攻阶段推进与提前切段逻辑，并没有被已经放置的动画 notify 对象驱动起来。
5. 当前行为仍然完全由 fallback 定时器驱动：
   `Attack_1` 的 advance window 大约在归一化时间 `0.714` 打开，原因为 `Reason=FallbackTime`
   `Attack_2` 的 advance window 大约在归一化时间 `0.703` 打开，原因为 `Reason=FallbackTime`
6. 因为 notify 回调根本没有触发，所以当前在编辑器里挪动这些 notify 对象的时机，并不会实际影响连段切换结果。
7. 当前最可能的根因是 notify 类型不匹配：
   现在放进去的标记是通过 `Add Notify -> New Notify...` 创建的；
   但代码监听的是 `OnPlayMontageNotifyBegin`，这条链路看起来需要真正的 montage notify / branching-point 风格事件。
8. 下一步探针方向应当是：
   把现有 phase 标记替换成真正的 `Montage Notify`，而不是继续使用 `New Notify...`
   优先处理 `CombatPhase_AdvanceNextSegment`、`CombatPhase_Recovery`、`CombatPhase_LogicEnded`
   将 `CombatPhase_AdvanceNextSegment` 设为 `Branching Point`
   然后重新跑一次 PIE，验证文件日志里是否出现：
   `MontageNotifyBegin`
   `MontageNotify`
   `AdvanceWindowOpened ... Reason=MontageNotify`

## 2026-05-26 Notify 修正执行单

### 当前根因

1. 现有代码监听的是 `UAnimInstance::OnPlayMontageNotifyBegin`。
2. 本轮日志已经证明运行时确实在播放：
   `/Game/Chinese_Warrior/Animations/Combat_Montage/AM_Melee_NormalAttackCombo.AM_Melee_NormalAttackCombo`
3. 但整轮跑测里没有任何：
   `MontageNotifyBegin`
   `MontageNotify`
4. 因此当前问题不是“代码没播到正确 Montage”，而是“当前 Montage 里的 phase / advance 标记类型不对，没走进这条 notify 回调链”。
5. 现阶段最可信的修正方向就是：
   把当前通过 `Add Notify -> New Notify...` 放进去的标记，替换成真正的 `Montage Notify`；
   其中 `CombatPhase_AdvanceNextSegment` 优先设成 `Branching Point`。

### 在 Unreal Editor 里的具体改法

1. 打开：
   `Content/Chinese_Warrior/Animations/Combat_Montage/AM_Melee_NormalAttackCombo`
2. 先找到当前已经放进去的普通攻击 phase / advance 标记。
   如果它们是通过 `Add Notify -> New Notify...` 创建的，先不要继续沿用。
3. 在 Montage 的 Notify 轨道上重新添加真正的 `Montage Notify`。
4. 需要优先补齐的名字：
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`
   `CombatPhase_AdvanceNextSegment`
5. `CombatPhase_AdvanceNextSegment` 至少在 `Attack_1` 和 `Attack_2` 两段都要有。
   `Attack_3` 没有后续段，通常不需要这个标记。
6. `CombatPhase_AdvanceNextSegment` 的细节里优先检查：
   `Montage Tick Type = Branching Point`
7. `CombatPhase_Recovery` 与 `CombatPhase_LogicEnded` 也建议一并改成真正的 `Montage Notify`，避免当前阶段推进继续只靠 fallback。
8. 名字必须和 C++ 完全一致，不能改大小写，也不要额外加前后缀：
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`
   `CombatPhase_AdvanceNextSegment`

### 摆放口径

1. `CombatPhase_AdvanceNextSegment`：
   放在“主要攻击表达已经完成，但还没进入明显硬收尾”的位置。
2. `CombatPhase_Recovery`：
   放在角色开始进入可读收招 / 可被后续规则判断为恢复阶段的位置。
3. `CombatPhase_LogicEnded`：
   放在这一段逻辑上已经算结束、只剩尾巴表现的位置。
4. 如果当前目标是先验证 notify 链路通不通，第一轮可以先不追求极致手感，只要顺序正确、时机大致合理即可。
5. 先让 notify 真正进回调，再微调位置；不要在“错误的 notify 类型”上反复挪时间点。

### 改完后的验证口径

1. 重新进 PIE，连续按普攻，观察日志里必须出现：
   `MontageNotifyBegin`
   `MontageNotify`
2. `AdvanceWindowOpened` 的原因应从：
   `Reason=FallbackTime`
   变成：
   `Reason=MontageNotify`
3. 若 `Recovery` / `LogicEnded` 也改对了，阶段推进日志应优先体现 notify 驱动，而不是只靠 fallback 定时器。
4. 若改完后仍完全没有 `MontageNotifyBegin`，优先重新检查：
   是否真的添加的是 `Montage Notify`
   是否改的是 `AM_Melee_NormalAttackCombo`
   是否保存了 Montage 和角色蓝图
   是否 PIE 前完成了最新编译 / 资源保存

### 本轮执行建议

1. 先只修 notify 类型，不先改 C++。
2. 第一轮至少把 `Attack_1`、`Attack_2` 的 `CombatPhase_AdvanceNextSegment` 换成正确类型。
3. 同轮把 `CombatPhase_Recovery`、`CombatPhase_LogicEnded` 一并换掉，减少闪避打断继续被 fallback 掩盖的干扰。
4. 跑完一次 PIE 后，把新的日志结果再对照：
   是否出现 `MontageNotifyBegin`
   是否出现 `AdvanceWindowOpened ... Reason=MontageNotify`
   `1 -> 2`、`2 -> 3` 是否明显早于旧的段尾衔接

## 2026-05-26 最终收口结论

1. 本 task 当前目标已完成：
   普攻 `1 -> 2 -> 3` 的切段不再依赖段尾 `FinishSegment`，而是由显式的 `CombatPhase_AdvanceNextSegment` notify 驱动。
2. 第一类问题最终确认为资源时机问题，不是代码继续绕过预输入规则：
   当 `CombatPhase_AdvanceNextSegment` 放得过早时，体感会像“连续点击后立刻连播两次普攻”；
   将 notify 调整到“主攻击表达完成后、但未进入硬收尾”的位置后，衔接体感恢复正常。
3. 第二类问题最终确认为视觉误判来源于 Montage 混合，不是段资源播错：
   新增日志已经证明 `Segment2` 运行时实际命中的底层动画是
   `AS_Melee_NormalAttack_02`
   而不是 `AS_Melee_NormalAttack_01`。
4. 第二段“看起来像第一段”的直接原因是：
   第二段起手被 Montage 的混合表现吞掉；
   将 Montage 的 `Blend In` 调小后，`AS_Melee_NormalAttack_02` 的上挑起手能够被正常读出。
5. 当前任务范围内的可交付结论是：
   `Montage Notify` 类型、notify 时机、以及 Montage 混合参数三者都已纳入正式联调口径。

## 本轮最终验证结果

1. 日志确认：
   `Segment1` 对应 `AS_Melee_NormalAttack_01`
   `Segment2` 对应 `AS_Melee_NormalAttack_02`
   `Segment3` 对应 `AS_Melee_NormalAttack_03`
2. `CombatPhase_AdvanceNextSegment` 已能够以 `Reason=MontageNotify` 打开切段窗口。
3. 经过资源时机和 Montage 混合参数调整后：
   `1 -> 2`
   `2 -> 3`
   的衔接体感恢复到当前任务可接受状态。
4. 仍存在的其他战斗问题，建议另起 task 单独处理，不与本任务继续混写。
