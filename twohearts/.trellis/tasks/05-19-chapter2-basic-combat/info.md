# 第二章基础战斗阶段总览

## 用途

1. 本文档用于在 Trellis 中承接第二章基础战斗模块当前阶段的高价值事实、主程序判断与下一阶段入口。
2. 它不是长期稳定规则真相源；长期稳定规则优先写入 `.trellis/spec/`。
3. 它的职责是让后续 workflow 拿到“当前到底已经做到哪里、下一步该拆什么、哪些旧判断已经过时”的准确信息。

## 2026-05-28 主程序更新后的阶段结论

1. 第二章基础战斗已经完成第一轮正式动作底座搭建，并完成了玩家防守侧的最小正式规则闭环。
2. 已正式落地的能力包括：
   普通攻击 Ability 正式承载；
   基础闪避；
   公共战斗语义层；
   预输入二期；
   最小敌对攻击探针；
   玩家侧正式命中 / 伤害 / 受击状态最小闭环；
   Guard 规则升级；
   Guard 结果结算；
   Guard 白盒验收。
3. 因此，当前阶段已经不再是“准备开始受击 / 格挡”，也不再是“05-28 那一批待做”。
4. 当前真实阶段应定义为：
   **玩家防守侧规则与调试闭环已经成立，但玩家输出到敌方的正式命中闭环、玩家受击正式表现、完整 Guard 表现与更稳定的战斗状态真相源仍未完成。**
5. 当前不应重开“基础 Guard 是否存在”“最小受击是否存在”“预输入二期是否存在”这类 task；下一阶段重点应转为补齐双向攻防与正式表现层。

## 对原始需求的差距判断

### 1. 玩家受击、伤害与状态

已完成：
1. 存在最小敌对攻击来源。
2. 玩家侧已经有正式的最小结果入口，而不是只剩临时日志。
3. Guard 已能在该入口上改写命中结果。

仍缺：
1. 正式受击动画 / Montage / 表现资源接线。
2. 更稳定的生命值 / 资源真相源，而不是长期停留在 receiver 组件内部。
3. 更复杂的受击分层规则，例如更稳定的 GuardBreak、后续更复杂打断等级。

### 2. 普通攻击与玩家输出

已完成：
1. `1 / 2 / 3` 三段正式 `Gameplay Ability` 承载。
2. `Startup / Active / Recovery / LogicEnded` 阶段语义。
3. 接入公共动作上下文。
4. 与 Dodge 的最小正式打断关系。
5. 连段连续性的一轮手感收口。

仍缺：
1. 普攻对敌方的正式命中派发。
2. 敌方侧最小正式受击 / 伤害 / 生命闭环。
3. 玩家侧攻击如何稳定驱动敌方受击样本，而不只是拥有攻击元数据模板。

### 3. 闪避

已完成：
1. `UE5 + GAS + C++` 正式承载。
2. 方向解析、`Montage + Root Motion + Notify`、冷却、无敌帧、动作结束链路与调试口径。
3. 冷却卡死问题已经单独收口。

仍缺或需要注意：
1. 闪避后移动表现如果继续优化，应优先走状态机自然衔接；`DodgeToRun` 额外资源切换已被证明不是正确主方向。
2. 当前闪避不是本阶段主阻塞项，不建议再把主线切回“基础闪避正式落地”。

### 4. 预输入

已完成：
1. 存在统一输入评估口径：`ExecuteNow / BufferInput / Reject`。
2. 普攻与 Dodge 已形成最小缓冲、消费与观察链路。
3. 白盒验证已经完成。

仍缺：
1. 当前主要不是系统是否存在，而是需要继续在更多动作与更多样本上回归验证。
2. 若后续加入更复杂 Guard / 受击分支，仍需继续观察“只保留最后一个合法输入”的口径是否稳定。

### 5. 格挡

已完成：
1. Guard 输入、Guard Ability 与动作上下文接入已经存在。
2. 最小窗口已经成立。
3. 面向最小敌对攻击探针的 Guard 成功 / 失败已经可观察、可解释、可白盒验证。

仍缺：
1. 不可格挡与 GuardBreak 的正式规则分支。
2. 正式攻击提示与结果提示体系，而不只是调试文字。
3. 完整 Guard 表现，包括资源配置、音效、VFX / 相机 / UI 反馈。
4. 更真实的位移、资源消费与反击消费侧，而不只是结果记录。

### 6. 战斗状态真相源

仍缺：
1. 玩家与敌方都可复用的更稳定生命值真相源。
2. Guard 资源 / 冷却 / 未来状态读取的更稳定承载。
3. 避免把越来越多战斗状态继续分散挂在各自 receiver 组件里。

## 已完成阶段与对应子 task

### 1. 基础闪避正式落地与收口

1. `05-19-dodge-second-pass-polish`
2. `05-21-dodge-resource-local-acceptance`
3. 结论：基础闪避正式版已经完成首轮落地，不再作为当前主阶段入口。

### 2. 公共战斗语义层

1. `05-21-combat-semantic-layer-readiness`
2. `05-23-combat-action-context-foundation`
3. `05-23-normal-attack-semantic-bridge`
4. `05-23-dodge-semantic-bridge-and-interrupt-unification`
5. `05-23-combat-input-evaluation-preinput-hook`
6. `05-23-combat-semantic-layer-test-pass`
7. `05-24-combat-semantic-layer-postarchive-alignment`
8. 结论：动作上下文、动作类型 / 阶段 / 结束原因、普攻与 Dodge 的统一动作承载已经形成首轮正式结构。

### 3. 最小预输入

1. `05-24-minimal-preinput-implementation`
2. `05-24-minimal-preinput-whitebox-test`
3. 结论：最小预输入已完成，但完整预输入系统仍是后续二期任务。

### 4. 局部手感与稳定性收口

1. `05-25-normal-attack-chain-continuity-polish`
2. `05-25-dodge-cooldown-stuck-fix`
3. `05-25-dodge-post-move-animation-switch`
4. 结论：
   普攻连段连续性已收口；
   Dodge 冷却卡死已修；
   `DodgeToRun` 方案判定为错误方向，不作为后续主线复用。

### 5. 受击 / 伤害前置闭环

1. `05-25-minimal-hostile-attack-probe`
2. `05-25-player-hit-damage-minimum-loop`
3. 结论：已经存在最小敌方来袭与玩家侧最小结果入口，但这还不是完整受击 / 伤害系统。

### 6. 基础 Guard 与白盒验收

1. `05-25-basic-guard-implementation`
2. `05-25-guard-feedback-and-whitebox-validation`
3. 结论：基础 Guard 已经正式存在，并完成了“逻辑成立、可观察、可验证”的阶段目标。

## 历史提交对照结论

1. `767ace6`：最小预输入正式消费链路落地。
2. `d520ff3`、`0ee0330`：最小敌对攻击探针与来袭接收入口落地。
3. `b0c2150`：玩家受击与伤害最小闭环建立。
4. `26076eb`：基础格挡正式实现接通。
5. `5593ba1`：基础格挡联调与白盒验收收口。
6. 这些提交与已归档 task 一起说明：当前真实状态已经明显晚于父 task 早期文档里的旧判断。

## 当前正确顺序

1. `05-28-normal-attack-hit-delivery-foundation`
   目标：
   让玩家普通攻击先形成正式命中派发，不再只有攻击元数据模板。
2. `05-28-hostile-hit-and-health-minimum-loop`
   目标：
   让最小敌方样本正式消费玩家普通攻击，建立“我打敌”的最小受击 / 扣血闭环。
3. `05-28-player-hitreaction-presentation-formalization`
   目标：
   把当前玩家受击状态升级成正式可见表现，而不是继续只停在逻辑与调试层。
4. `05-28-guard-unguardable-and-guardbreak-rules`
   目标：
   补齐不可格挡与 GuardBreak 结果分支，为完整 Guard 表现提供稳定输入。
5. `05-28-full-guard-presentation-implementation`
   目标：
   补齐 Guard 的美术资产配置、音效与反馈，让 Guard 从工程验证能力升级成正式表现能力。
6. `05-28-combat-attribute-truthsource-consolidation`
   目标：
   收口生命值与未来 Guard 资源语义的真相源，避免双向攻防建立后继续各写一套状态。
7. `05-28-combat-sample-expansion-and-regression`
   目标：
   扩展攻防样本与白盒验收覆盖，让后续收口不再只依赖单一 probe 样本。

## 当前阶段入口判断

1. 若 `05-28-normal-attack-hit-delivery-foundation` 尚未完成：
   当前下一直接开工项就是它，不建议直接跳到敌方受击、Guard 表现或回归扩展。
2. 若 `05-28-normal-attack-hit-delivery-foundation` 已完成，但 `05-28-hostile-hit-and-health-minimum-loop` 未完成：
   当前仍处于“把双向攻防最小闭环补齐”的阶段，不提前切到完整 Guard 表现。
3. `05-28-full-guard-presentation-implementation` 不建议早于 `05-28-guard-unguardable-and-guardbreak-rules`：
   否则表现分支输入会持续变化。
4. `05-28-combat-sample-expansion-and-regression` 默认放在上述主要逻辑与表现 task 之后，不作为当前第一开工项。

## 当前不应再误判的点

1. 当前不是“还没进入 Guard”的阶段。
2. 当前也不是“Guard 已完成，第二章基础战斗已经收官”的阶段。
3. 当前真正缺的是：
   玩家普攻打到敌方；
   玩家受击正式表现；
   不可格挡 / GuardBreak；
   完整 Guard 表现；
   更稳定的战斗状态真相源；
   多样本回归覆盖。
4. 不建议再重开“基础 Guard 正式实现”或“最小敌对攻击探针”这种已经完成首轮验收的 task。
5. 不建议把闪避后移动表现问题误判为当前主线阻塞。

## 当前关键代码 / 系统落点

1. 公共动作上下文：
   `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
2. 玩家角色输入与调试承载：
   `Source/twohearts/twoheartsCharacter.*`
   `Source/twohearts/twoheartsDebugHUD.cpp`
3. 普攻 Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
4. Dodge Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
5. Guard Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
6. 最小敌对攻击与玩家来袭结果：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
7. 攻击元数据与普通攻击输出入口：
   `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`

## 来源

1. `h:/twohearts/docs/a双心印战斗系统框架.md`
2. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-24-minimal-preinput-implementation/prd.md`
3. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-24-minimal-preinput-whitebox-test/report.md`
4. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-25-normal-attack-chain-continuity-polish/info.md`
5. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-25-minimal-hostile-attack-probe/info.md`
6. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-25-player-hit-damage-minimum-loop/info.md`
7. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-25-basic-guard-implementation/info.md`
8. `h:/twohearts/twohearts/.trellis/tasks/archive/2026-05/05-25-guard-feedback-and-whitebox-validation/whitebox-acceptance-2026-05-27.md`
9. `git log --oneline -n 40`
