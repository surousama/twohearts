# 最小敌对攻击探针

## Goal

作为 `05-19-chapter2-basic-combat` 之下、面向“格挡前置闭环”的首个执行子 task，建立一个最小但稳定的敌对攻击来源，让程序与测试可以在当前 `UE5 + GAS + C++` 基础战斗架构上，实际观察“敌方发起攻击 -> 玩家接收来袭 -> 攻击时机可读”的最小链路，为后续受击/伤害和格挡正式落地提供可复用入口。

## What I already know

* 当前第二章已完成普通攻击、基础闪避、公共战斗语义层、最小预输入的首轮正式落地。
* 当前有一个局部手感收口子 task `05-25-normal-attack-chain-continuity-polish` 正在进行，但它不改变第二章后续主顺序。
* 现有战斗动作上下文已经正式落在 `UTwoHeartsCombatActionContextComponent`，动作类型中已预留 `Guard`。
* 输入枚举 `ETwoHeartsAbilityInputID` 已预留 `Guard`，动作状态 Tag 也已存在 `State.Action.Guard`。
* 当前代码里尚未看到最小敌人、敌方攻击动作、玩家受击结果、命中提示或可被格挡的来袭事件正式落地。
* 因此，格挡当前真正缺的不是“有没有 Guard 枚举”，而是“有没有一个最小但可验收的敌方攻击来源”。

## Requirements

* 本 task 必须提供一个最小敌对攻击探针，能让开发者在本地稳定复现“敌方发起一次可观察攻击”的过程。
* 该探针可以是最简敌人 Actor、测试木桩敌人、临时攻击发射器或其他当前阶段最低成本方案，但必须是可反复使用的正式调试入口，而不是一次性手工拼蓝图。
* 攻击探针至少需要具备：
  可放置到关卡；
  可触发一次明确攻击；
  攻击存在可读前摇或启动提示；
  攻击存在明确的命中时机；
  攻击结束后能够恢复到可再次测试的状态。
* 本轮不要求敌人具备完整 AI、寻路、追击、状态机、完整战斗循环；允许采用手动触发、定时触发或距离触发等最低成本方式。
* 攻击结果在本 task 内不要求完成“真正扣血/真正格挡结算”，但必须把“来袭已经发生、命中窗口已到”的事件以稳定方式暴露给玩家侧后续系统使用。
* 这个暴露方式可以是：
  命中回调；
  overlap / trace 命中事件；
  统一攻击描述结构；
  或其他后续 `player-hit-damage-minimum-loop` 能正式接上的接口。
* 攻击提示必须至少具备一种当前阶段可读手段：
  动画前摇；
  Niagara / 粒子；
  材质闪光；
  调试形状与日志；
  或明确的屏幕调试输出。
* 本 task 的交付重点不是“敌人做得像不像正式内容”，而是“为后续受击与格挡提供稳定、重复、低成本的攻击样本”。
* 若程序实现过程中发现“单轮把基础敌人承载、攻击动作、提示特效”全部做完明显超出单轮可验收范围，则允许保留最小正式版本，但必须保证：
  仍有一个实际可播放的攻击；
  仍有一个明确的命中时机；
  仍可被后续 task 直接复用。

## Acceptance Criteria

* [ ] 关卡中存在一个可重复使用的最小敌对攻击探针，开发者可稳定触发其攻击。
* [ ] 攻击具有明确的开始、命中时机和结束，不是无时序的瞬时占位逻辑。
* [ ] 至少有一种可读提示帮助观察攻击即将到来或已经进入命中阶段。
* [ ] 攻击命中时机已通过日志、调试绘制、回调或结构化事件暴露给玩家侧后续系统。
* [ ] 本 task 产物可被 `05-25-player-hit-damage-minimum-loop` 直接复用，而不需要推翻重做。
* [ ] 本 task 没有越界扩散到完整敌人 AI、完整受击系统、完整格挡系统或完整数值系统。

## Out of Scope

* 完整敌人行为树、感知、追击、索敌和战斗 AI
* 完整伤害数值、血量系统、死亡流程
* 玩家正式受击反馈与硬直系统
* Guard 输入、Guard Ability 和正式格挡判定
* 面向所有未来敌人的通用攻击框架一次性做全

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 当前在第二章主顺序中的位置：
  它是“格挡入口规划”下的第一个前置执行项，
  本质上为“受击与伤害”以及“格挡”共同提供最小敌方来袭来源。
* 与后续任务的依赖关系：
  `05-25-player-hit-damage-minimum-loop` 依赖本 task 提供的最小攻击来源；
  `05-25-basic-guard-implementation` 不应绕过本 task 直接开工。
* 当前建议优先复用或观察的代码落点：
  `Source/twohearts/twoheartsCharacter.cpp`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
* 建议程序优先确认的实现问题：
  攻击探针是用最小 Actor 还是最小 Character 承载；
  攻击命中时机是用 Montage Notify、Trace 时窗还是简化 Trigger；
  给后续玩家侧的来袭信息最少需要包含哪些字段。
* 若程序开工时仍缺信息，应优先回问：
  攻击探针更偏向“站桩测试桩”还是“最小 humanoid 敌人”；
  当前是否已有可直接复用的攻击动画资产；
  攻击提示更希望先用调试提示还是尽早接近正式特效。
* 当前允许的过渡实现边界：
  允许先用单段攻击、单方向攻击、调试提示替代正式内容；
  但不允许把整个攻击来源写成只靠手工按钮和临时代码、无法复用的纯一次性验证脚本。
* 对应上游判断来源：
  `05-19-chapter2-basic-combat/info.md` 中的“2026-05-25 格挡阶段入口评估”。

