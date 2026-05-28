# 格挡规则基础升级实现记录

## 本轮实际完成

1. 将 Guard 从“命中后按最小窗口重写结果”升级为“按攻击段规则先判定、再进入正式格挡分支”。
2. 在攻击元数据中补齐最小 Guard 规则承载：
   - 是否可格挡
   - 最大格挡距离
   - 最大高度差容忍
   - 格挡朝向半角
3. 让当前 `HostileAttackProbe` 样本可直接配置上述 Guard 规则，而不是只写死在 Guard Ability 里。
4. 将 Guard 判定接到 `AttackContact` 前置事件，先做规则评估，再落正式 `PlayerHitResult / PlayerDamageResult`。
5. 补齐 Guard 成功 / 失败 / 不可格挡的调试事件，至少可区分：
   - `GuardFailedTooEarly`
   - `GuardFailedTooLate`
   - `GuardFailedDistance`
   - `GuardFailedHeight`
   - `GuardFailedAngle`
   - `GuardAttackUnguardable`
   - `GuardRuleSuccess`
6. 修正 Guard 成功时的正式落账顺序：
   - 先记录 `GuardRewriteQueued`
   - 再将本次 `PlayerHitResult` 正式落成 `GuardRewritten`
   - 再将 `PlayerDamageResult` 正式落成 `GuardBlocked`
   - 最终生命值保持不变，不进入新的受击状态
7. 补充了 Guard 的 `DrawDebug` 扇区与攻击来向连线，便于 PIE 里直接观察当前角度与距离判定。
8. HUD 现在可直接显示当前攻击样本的 Guard 规则数值：
   - `guard`
   - `dist`
   - `angle`
   - `guard_height`

## 主要代码落点

1. `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h`
3. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.cpp`
4. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
5. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
6. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.h`
7. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.cpp`
8. `Source/twohearts/twoheartsDebugHUD.cpp`

## 当前规则口径

1. 当前默认 Guard 规则参数为：
   - 距离 `220.0`
   - 高度差容忍 `120.0`
   - 朝向半角 `100.0`
2. Guard 成功必须同时满足：
   - 攻击段本身可格挡
   - Guard 当前仍在有效窗口
   - 距离满足
   - 高度差满足
   - 朝向角度满足
3. Guard 成功继续沿用正式链路：
   `AttackContact -> PlayerHitResult(GuardRewritten) -> PlayerDamageResult(GuardBlocked)`
4. 本轮仍然以单一 `HostileAttackProbe` 样本完成规则闭环，但规则承载已经不再写死成只适配一个样本。

## 本轮验证

1. 已多次执行 `twoheartsEditor Win64 Development` 构建，结果均为 `Succeeded`。
2. 已完成多轮 PIE 白盒验证，并确认以下成功样本链路稳定出现：
   - `GuardRuleEvaluate`
   - `GuardRewriteQueued`
   - `GuardRuleSuccess`
   - `PlayerHitResult = GuardRewritten`
   - `PlayerDamageResult = GuardBlocked`
3. 已通过日志确认成功格挡时：
   - `final=0.00`
   - `health_before` 与 `health_after` 保持一致
   - 不再进入新的受击链
4. 已通过日志确认失败分支仍然有效，至少出现过：
   - `GuardFailedTooLate`
   - `GuardFailedAngle`

## 已知限制

1. 当前 `DrawDebug` 仍是偏工程调试视图，不是最终表现层可视化。
2. 当前尚未扩展到更复杂的不可格挡攻击类型、GuardBreak 分支或完整收益结算。
3. 当前尚未把规则配置推广到更多敌对攻击来源或数据资产体系。

## Spec 判断

1. 本轮已完成一次 spec 回写判断。
2. 当前结论是：暂不回写 `.trellis/spec/`。
3. 原因：
   - 本轮主要沉淀的是当前 Guard 规则闭环实现与接线细节；
   - 这些经验已足够落入 task 留档；
   - 是否要把“AttackContact 前置广播 + Guard 预登记改写”提升为长期规范，建议等后续更多攻击来源接入后再统一抽象。
