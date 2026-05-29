# 第二章基础战斗模块推进

## Goal

作为第二章基础战斗模块的父 task，持续维护当前阶段事实、原始需求差距、正确推进顺序和下一批子 task 拆分，供后续主程序、资深程序与测试直接复用。

## What I already know

* 原始需求历史来源是旧文档 `a双心印战斗系统框架.md`；当前仓库内可直接对照的迁移口径主要在 `.trellis/spec/combat/system-framework.md`
* `05-28` 这一批子 task 已全部归档完成：攻击元数据、玩家伤害结果、玩家受击最小实现、Guard 规则升级、Guard 结果结算、预输入二期泛化都已经形成一轮正式代码落地
* 当前真实阶段已经晚于旧口径中的“最小预输入之后准备开始受击 / 格挡”，也晚于父 task 中曾经写过的“下一步先做 05-28-*”
* 当前阶段更准确的定义是：
  玩家防守侧的规则与调试闭环已经成立；
  但玩家输出到敌方的正式命中闭环、玩家受击正式表现、完整 Guard 表现与更稳定的战斗状态真相源仍未完成
* 结合现有代码与归档 task，当前主要差距集中在：
  普通攻击正式命中派发；
  敌方受击与生命最小闭环；
  玩家受击表现正式化；
  不可格挡与 GuardBreak 规则；
  完整 Guard 表现；
  攻防样本扩展与更稳定的战斗状态真相源
* `05-25-dodge-post-move-animation-switch` 已证明“额外补一套 DodgeToRun 资源切换”不是正确主方向；闪避后表现若再收口，应优先走状态机自然衔接，而不是重开同类 task
* 这一轮父 task 的职责仍然是维护阶段真相源和主程序顺序判断，并继续拆出可直接开工的执行子 task，而不是直接承担实现

## Requirements

* 作为阶段级父 task 使用，不直接承担单轮代码或资源实现
* 必须持续对照原始战斗框架口径、当前 archive / git 记录与真实代码实现，维护“已完成什么、还差什么、为什么这样排序”
* 必须明确给出“下一直接开工子 task”与“哪些 task 仍不能提前并行”的主程序判断
* 新一轮子 task 必须围绕当前真实主顺序展开：
  `normal-attack-hit-delivery-foundation`
  -> `hostile-hit-and-health-minimum-loop`
  -> `player-hitreaction-presentation-formalization`
  -> `guard-unguardable-and-guardbreak-rules`
  -> `full-guard-presentation-implementation`
  -> `combat-attribute-truthsource-consolidation`
  -> `combat-sample-expansion-and-regression`
* 父 task 需要同步回写高度凝练且不过时的阶段信息，避免后续 workflow 继续沿用“还没到 Guard”之类旧判断
* 稳定规则仍应优先回收至 `.trellis/spec`；阶段现状、排序、入口判断优先维护在父 task 文档
* 新任务中必须至少有一个子 task 专门覆盖完整格挡表现实现，并写清：
  格挡相关美术资产配置；
  格挡音效；
  格挡成功 / 失败 / 不可格挡 / GuardBreak 的反馈链路

## Acceptance Criteria

* [ ] 父 task 文档已经从“05-28 那一批待做”更新到“05-28 那一批已完成、当前进入下一批完整化阶段”的真实状态
* [ ] 下一阶段的多个子 task 已经按依赖顺序拆出，并能直接继续推进
* [ ] 当前开发进度信息已同步到至少一份对外总览文档，后续 workflow 不会再拿到明显过时的阶段判断
* [ ] 父 task 中已明确哪些原始需求已经形成最小闭环，哪些仍是当前主缺口
* [ ] 已明确当前下一直接开工项是 `05-28-normal-attack-hit-delivery-foundation`，并说明为什么不能直接跳到完整 Guard 表现或更后面的回归扩展
* [ ] 已创建一项专门覆盖完整格挡表现的子 task，且任务要求中明确包含美术资产配置、格挡音效与格挡反馈

## Out of Scope

* 单轮具体代码实现
* 某个单点 bug 的直接修复方案
* 完整战斗框架 spec 一次性重写
* 直接替代各子 task 的实现 PRD

## Technical Notes

* 原始需求迁移口径：`.trellis/spec/combat/system-framework.md`
* 关键已完成子 task：
  * `05-19-dodge-second-pass-polish`
  * `05-21-dodge-resource-local-acceptance`
  * `05-21-combat-semantic-layer-readiness`
  * `05-23-normal-attack-weapon-switch`
  * `05-24-combat-semantic-layer-postarchive-alignment`
  * `05-24-minimal-preinput-implementation`
  * `05-24-minimal-preinput-whitebox-test`
  * `05-25-dodge-cooldown-stuck-fix`
  * `05-25-normal-attack-chain-continuity-polish`
  * `05-25-minimal-hostile-attack-probe`
  * `05-25-player-hit-damage-minimum-loop`
  * `05-25-basic-guard-implementation`
  * `05-25-guard-feedback-and-whitebox-validation`
* 新拆下一阶段子 task：
  * `05-28-attack-metadata-foundation`
  * `05-28-player-damage-result-formalization`
  * `05-28-player-hit-reaction-minimum-implementation`
  * `05-28-guard-rule-foundation-upgrade`
  * `05-28-guard-outcome-settlement-and-counter-hook`
  * `05-28-preinput-second-pass-generalization`
* 新拆下一阶段子 task：
  * `05-28-normal-attack-hit-delivery-foundation`
  * `05-28-hostile-hit-and-health-minimum-loop`
  * `05-28-player-hitreaction-presentation-formalization`
  * `05-28-guard-unguardable-and-guardbreak-rules`
  * `05-28-full-guard-presentation-implementation`
  * `05-28-combat-attribute-truthsource-consolidation`
  * `05-28-combat-sample-expansion-and-regression`
* 当前关键代码落点：
  * `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
  * `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  * `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  * `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  * `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  * `Source/twohearts/twoheartsCharacter.*`
  * `Source/twohearts/twoheartsDebugHUD.cpp`
* 当前阶段判断：
  * 已完成：基础动作正式化 + 玩家防守侧最小规则闭环 + Guard 规则与结算 + 预输入二期
  * 当前应进入：玩家输出到敌方的正式命中闭环、玩家受击正式表现、不可格挡 / GuardBreak、完整 Guard 表现与战斗状态真相源收口
  * 下一直接开工：`05-28-normal-attack-hit-delivery-foundation`
  * 暂不建议直接跳过前置去做：完整格挡表现、最终回归扩展、全面属性系统重构
