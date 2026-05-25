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
