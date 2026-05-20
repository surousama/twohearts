# 第二章基础战斗模块推进

## Goal

作为第二章基础战斗模块的父 task，承载当前阶段背景、正确开发顺序和阶段性结论，供后续多个子 task 复用。

## What I already know

* 当前第二章推进顺序已经明确
* 当前主线仍是：基础闪避正式落地 -> 公共战斗语义层 -> 最小预输入
* 普攻 `AbilitySystem` 正式迁移已完成

## Requirements

* 作为阶段级父 task 使用，不直接承担单轮实现
* 子 task 必须围绕当前主顺序展开
* 稳定阶段结论应回收至 `.trellis/spec` 或主文档

## Acceptance Criteria

* [ ] 后续子 task 能围绕本 task 的阶段背景推进
* [ ] 阶段顺序不被破坏

## Out of Scope

* 单轮具体实现细节
* 某个单点 bug 的直接修复方案

## Technical Notes

* 对应旧文档：`../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
