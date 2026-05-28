# 第二章基础战斗模块推进

## Goal

作为第二章基础战斗模块的父 task，持续维护当前阶段事实、原始需求差距、正确推进顺序和下一批子 task 拆分，供后续主程序、资深程序与测试直接复用。

## What I already know

* 原始需求来源是 `h:/twohearts/docs/a双心印战斗系统框架.md`
* 结合已归档子 task 与最近提交，当前已经完成首轮正式落地：
  普通攻击
  -> 基础闪避
  -> 公共战斗语义层
  -> 最小预输入
  -> 最小敌对攻击探针
  -> 玩家侧最小受击结果
  -> 基础 Guard
  -> Guard 白盒验收
* 当前真实阶段不再是“最小预输入之后准备开始受击与伤害 / 格挡”，而是“最小攻防闭环已经成立，但完整受击/伤害与完整格挡规则仍未完成”
* 当前主要差距集中在：
  攻击描述与命中元数据；
  正式伤害结果；
  正式受击状态；
  格挡规则二期；
  格挡结果结算；
  预输入二期泛化
* `05-25-dodge-post-move-animation-switch` 已证明“额外补一套 DodgeToRun 资源切换”不是正确主方向；闪避后表现若再收口，应优先走状态机自然衔接，而不是重开同类 task
* 这一轮父 task 的职责是更新阶段真相源，并继续拆出下一批可直接开工的子 task，而不是直接承担实现

## Requirements

* 作为阶段级父 task 使用，不直接承担单轮代码或资源实现
* 必须持续对照 `a双心印战斗系统框架.md` 与当前 archive / git 记录，维护“已完成什么、还差什么、为什么这样排序”
* 必须明确给出“下一直接开工子 task”与“哪些 task 仍不能提前并行”的主程序判断
* 新一轮子 task 必须围绕当前真实主顺序展开：
  `attack-metadata-foundation`
  -> `player-damage-result-formalization`
  -> `player-hit-reaction-minimum-implementation`
  -> `guard-rule-foundation-upgrade`
  -> `guard-outcome-settlement-and-counter-hook`
  -> `preinput-second-pass-generalization`
* 父 task 需要同步回写高度凝练且不过时的阶段信息，避免后续 workflow 继续沿用“还没到 Guard”之类旧判断
* 稳定规则仍应优先回收至 `.trellis/spec`；阶段现状、排序、入口判断优先维护在父 task 文档

## Acceptance Criteria

* [ ] 父 task 文档已经从“最小预输入之后”更新到“最小攻防闭环 + 基础 Guard 已完成”的真实阶段
* [ ] 下一阶段的多个子 task 已经按依赖顺序拆出，并能直接继续推进
* [ ] 当前开发进度信息已同步到至少一份对外总览文档，后续 workflow 不会再拿到明显过时的阶段判断
* [ ] 父 task 中已明确哪些原始需求已经形成最小闭环，哪些仍是当前主缺口
* [ ] 已明确当前下一直接开工项是 `05-28-attack-metadata-foundation`，并说明为什么不能直接跳到更后面的 Guard 二期或完整预输入

## Out of Scope

* 单轮具体代码实现
* 某个单点 bug 的直接修复方案
* 完整战斗框架 spec 一次性重写
* 直接替代各子 task 的实现 PRD

## Technical Notes

* 原始需求：`h:/twohearts/docs/a双心印战斗系统框架.md`
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
* 当前阶段判断：
  * 已完成：基础动作正式化 + 最小攻防闭环 + 基础 Guard 白盒验收
  * 当前应进入：受击与伤害正式化、格挡规则二期、更完整公共规则收束
  * 下一直接开工：`05-28-attack-metadata-foundation`
  * 暂不建议直接跳过前置去做：完整受击动画系统、完整格挡结算、完整预输入泛化
