# 第二章基础战斗阶段总览

## 用途

1. 本文档用于在 Trellis 中承接第二章基础战斗模块当前阶段的高价值事实、主程序判断与下一阶段入口。
2. 它不是长期稳定规则真相源；长期稳定规则优先写入 `.trellis/spec/`。
3. 它的职责是让后续 workflow 拿到“当前到底已经做到哪里、下一步该拆什么、哪些旧判断已经过时”的准确信息。

## 2026-05-28 主程序更新后的阶段结论

1. 第二章基础战斗已经完成第一轮正式动作底座搭建，并且已经跑通最小攻防闭环。
2. 已正式落地的能力包括：
   普通攻击；
   基础闪避；
   公共战斗语义层；
   最小预输入；
   最小敌对攻击探针；
   玩家侧最小受击结果入口；
   基础 Guard；
   Guard 白盒验收。
3. 因此，当前阶段**不再**是“最小预输入之后、准备开始受击与伤害 / 格挡”的旧状态。
4. 当前真实阶段应定义为：
   **最小攻防闭环已经成立，但完整受击 / 伤害系统与完整格挡规则系统仍未完成。**
5. 当前不应重开“基础 Guard 是否存在”这一类 task；下一阶段重点应转为把现有最小样本升级成符合原始框架文档的规则型系统。

## 对原始需求的差距判断

### 1. 受击与伤害

已完成：
1. 存在最小敌对攻击来源。
2. 玩家侧已经有正式的最小结果入口，而不是只剩临时日志。
3. Guard 已能在该入口上改写命中结果。

仍缺：
1. 正式伤害结果与正式血量承载。
2. 角色进入受击状态、受击恢复、受击期间动作打断规则。
3. 按来袭方向决定受击方向或至少提供稳定方向数据。
4. “受击反应类型”与“伤害 / 机制标签类型”的正式分层承载。
5. 从“HitConfirmed”升级到“伤害 + 受击 + 恢复”的完整玩家侧流程。

### 2. 普通攻击

已完成：
1. `1 / 2 / 3` 三段正式 `Gameplay Ability` 承载。
2. `Startup / Active / Recovery / LogicEnded` 阶段语义。
3. 接入公共动作上下文。
4. 与 Dodge 的最小正式打断关系。
5. 连段连续性的一轮手感收口。

仍缺：
1. 普攻攻击段自身的正式攻击描述与命中元数据。
2. 普攻与正式伤害 / 受击系统的规则层接线。
3. 普攻攻击段上的可格挡、可闪避规避、受击反应类型等元数据承载。

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
1. 还不是面向所有允许缓存动作的完整预输入系统。
2. 还没有稳定落实“只保留最后一个合法输入”的统一规则。
3. 还没有角色默认窗口 + 动作覆写的完整配置化窗口体系。
4. 消费失败后的回退 / 保留 / 清槽策略仍需要二期收口。
5. 被受击或强制中断时的统一清空口径还未正式扩展到更完整动作集合。

### 5. 格挡

已完成：
1. Guard 输入、Guard Ability 与动作上下文接入已经存在。
2. 最小窗口已经成立。
3. 面向最小敌对攻击探针的 Guard 成功 / 失败已经可观察、可解释、可白盒验证。

仍缺：
1. 攻击段“可格挡 / 不可格挡”的正式规则承载。
2. 黄 / 红提示这类正式攻击提示体系。
3. 位置、角度、距离条件，而不只是最小时间窗。
4. 格挡成功后的位移结果与伤害结果二层结算。
5. 格挡成功后的资源、冷却、反击接口预留。
6. 从“最小窗口改写命中结果”升级到“完整格挡规则系统”。

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

1. `05-28-attack-metadata-foundation`
   目标：
   建立统一攻击描述与命中元数据，让后续伤害、受击、格挡不再各写一套临时判断。
2. `05-28-player-damage-result-formalization`
   目标：
   把当前最小命中结果升级成正式伤害结果与最小血量承载。
3. `05-28-player-hit-reaction-minimum-implementation`
   目标：
   建立玩家受击状态、自动恢复与最小方向数据承载。
4. `05-28-guard-rule-foundation-upgrade`
   目标：
   把 Guard 从“最小窗口改写命中结果”升级为“攻击段规则 + 时机 + 位置条件”的基础格挡规则系统。
5. `05-28-guard-outcome-settlement-and-counter-hook`
   目标：
   补齐格挡成功后的位移结果、伤害结果、资源 / 冷却语义和反击接口预留。
6. `05-28-preinput-second-pass-generalization`
   目标：
   在攻防规则层稳定后，继续把预输入扩展成更完整、可配置、可清理的统一系统。

## 当前阶段入口判断

1. 若 `05-28-attack-metadata-foundation` 尚未完成：
   当前下一直接开工项就是它，不建议跳到后面的伤害、受击或 Guard 二期。
2. 若 `05-28-attack-metadata-foundation` 已完成，但 `05-28-player-damage-result-formalization` 未完成：
   当前仍处于“受击与伤害正式化”阶段，不进入 Guard 规则升级。
3. 只有当 `05-28-player-hit-reaction-minimum-implementation` 已经给出稳定的玩家侧受击状态入口后，才适合进入 `05-28-guard-rule-foundation-upgrade`。
4. 只有当 Guard 规则升级与 Guard 结果结算都完成后，才建议把主线切到 `05-28-preinput-second-pass-generalization`。

## 当前不应再误判的点

1. 当前不是“还没进入 Guard”的阶段。
2. 当前也不是“Guard 已完成，第二章基础战斗已经收官”的阶段。
3. 当前真正缺的是：
   正式伤害；
   正式受击；
   格挡规则二期；
   格挡结果结算；
   预输入二期泛化。
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
