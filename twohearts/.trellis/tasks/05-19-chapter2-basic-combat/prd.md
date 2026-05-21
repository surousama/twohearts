# 第二章基础战斗模块推进

## Goal

作为第二章基础战斗模块的父 task，承载当前阶段背景、正确开发顺序和阶段性结论，供后续多个子 task 复用。

## What I already know

* 当前第二章推进顺序已经明确
* 当前主线仍是：基础闪避正式落地 -> 公共战斗语义层 -> 最小预输入
* 普攻 `AbilitySystem` 正式迁移已完成
* 当前唯一进行中的子 task 仍是 `05-19-dodge-second-pass-polish`
* 仅完成“闪避方向冲突修复”还不等于“基础闪避阶段可验收收口”

## Requirements

* 作为阶段级父 task 使用，不直接承担单轮实现
* 子 task 必须围绕当前主顺序展开
* 稳定阶段结论应回收至 `.trellis/spec` 或主文档
* 父 task 需要持续给出“下一直接开工子 task”和“是否允许进入下一阶段”的主程序判断

## Acceptance Criteria

* [ ] 后续子 task 能围绕本 task 的阶段背景推进
* [ ] 阶段顺序不被破坏
* [ ] 当前阶段至少拆出：闪避问题收口、闪避资源联调验收、公共语义层开工评估

## Out of Scope

* 单轮具体实现细节
* 某个单点 bug 的直接修复方案

## Technical Notes

* 对应旧文档：`../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
* 当前子 task：
  * `05-19-dodge-second-pass-polish`
  * `05-21-dodge-resource-local-acceptance`
  * `05-21-combat-semantic-layer-readiness`
