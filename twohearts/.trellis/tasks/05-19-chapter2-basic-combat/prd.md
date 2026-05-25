# 第二章基础战斗模块推进

## Goal

作为第二章基础战斗模块的父 task，承载当前阶段背景、正确开发顺序和阶段性结论，供后续多个子 task 复用。

## What I already know

* 当前第二章推进顺序已经明确
* 当前主线顺序已经从“基础闪避正式落地 -> 公共战斗语义层 -> 最小预输入”推进完成到下一观察点，后续正式大阶段仍应按：
  最小预输入之后的局部手感/表现收口
  -> 受击与伤害
  -> 格挡与更完整公共规则
* 普攻 `AbilitySystem` 正式迁移已完成
* 基础闪避正式落地、公共战斗语义层、最小预输入及其白盒跑测都已经拆成子 task 并推进完成
* 当前进行中的子 task 已切换为 `05-25-dodge-post-move-animation-switch`
* 当前新增问题不是底座阻断，而是 Dodge 结束后在存在移动输入时仍沿用“原地起身”资产，导致衔接移动表现僵硬
* 这一轮需求属于最小预输入跑通后的局部表现收口，适合继续叠加在现有 `UE5 + GAS + C++` Dodge 架构上解决，不需要回退到底层重做

## Requirements

* 作为阶段级父 task 使用，不直接承担单轮实现
* 子 task 必须围绕当前主顺序展开
* 稳定阶段结论应回收至 `.trellis/spec` 或主文档
* 父 task 需要持续给出“下一直接开工子 task”和“是否允许进入下一阶段”的主程序判断
* 当主线阶段已推进时，父 task 需要同步维护“哪些基础能力已跑通、当前仍在收口什么、为什么还没有进入下一正式大阶段”

## Acceptance Criteria

* [ ] 后续子 task 能围绕本 task 的阶段背景推进
* [ ] 阶段顺序不被破坏
* [ ] 已完成阶段与当前进行中阶段在父 task 中保持可读，不再停留在旧状态
* [ ] 当前阶段至少已覆盖：闪避问题收口、闪避资源联调验收、公共语义层开工评估、最小预输入实施与白盒跑测

## Out of Scope

* 单轮具体实现细节
* 某个单点 bug 的直接修复方案

## Technical Notes

* 对应旧文档：`../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
* 当前子 task：
  * `05-19-dodge-second-pass-polish`
  * `05-21-dodge-resource-local-acceptance`
  * `05-21-combat-semantic-layer-readiness`
  * `05-23-normal-attack-weapon-switch`
  * `05-24-combat-semantic-layer-postarchive-alignment`
  * `05-24-minimal-preinput-implementation`
  * `05-24-minimal-preinput-whitebox-test`
  * `05-25-dodge-post-move-animation-switch`
* 当前阶段判断：
  * 已完成的正式主链路：基础闪避正式落地、公共战斗语义层、最小预输入
  * 当前进行中的局部收口：Dodge 后移动输入存在时的动画衔接资源切换
  * 进入“受击与伤害”之前，允许先把这类直接影响基础战斗手感、且不破坏阶段边界的局部问题收干净
