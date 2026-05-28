# 开发日志 - palladianli（第 1 部分）

> AI 开发会话日志
> 开始于：2026-05-19

---



## Session 1: Finish dodge-second-pass-polish

**Date**: 2026-05-21
**Task**: Finish dodge-second-pass-polish
**Branch**: `master`

### Summary

Unified dodge direction with current facing, verified editor build, and archived the dodge second-pass polish task.

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `fd41608` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 2: 普通攻击语义桥接收尾

**Date**: 2026-05-23
**Task**: 普通攻击语义桥接收尾
**Branch**: `master`

### Summary

完成普通攻击语义桥接到公共动作上下文，补充 combat spec 接入约定，并归档当前子任务。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `5024f1a` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 3: 基础闪避语义桥接与打断统一

**Date**: 2026-05-23
**Task**: 基础闪避语义桥接与打断统一
**Branch**: `master`

### Summary

完成 Dodge 接入公共动作上下文，并将普攻被 Dodge 打断升级为公共层统一接入口；已通过 twoheartsEditor 构建验证。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `eb0a444` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 4: Combat input evaluation polish wrap-up

**Date**: 2026-05-23
**Task**: Combat input evaluation polish wrap-up
**Branch**: `master`

### Summary

Refined combat input evaluation by separating evaluation result from consumption route, split Character-side input handling into clearer execution paths, verified with a successful twoheartsEditor build, and archived the subtask.

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `ede54cd` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 5: 最小预输入正式实施

**Date**: 2026-05-24
**Task**: 最小预输入正式实施
**Branch**: `master`

### Summary

完成最小预输入正式链路，打通普攻晚窗与 Dodge 后续输入的缓冲消费，并通过 UE 5.6 Editor Development 构建验证。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `767ace6` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 6: Close minimal preinput whitebox test

**Date**: 2026-05-25
**Task**: Close minimal preinput whitebox test
**Branch**: `master`

### Summary

Validated that the dodge failure report was caused by PIE loading an outdated editor module, rebuilt twoheartsEditor with H:\UE_5.6, confirmed dodge and minimal preinput in PIE, updated the senior programmer guardrails, and archived the whitebox test task.

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `6989ad6` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 7: Normal attack weapon visual switching

**Date**: 2026-05-25
**Task**: Normal attack weapon visual switching
**Branch**: `master`

### Summary

Implemented character-side weapon visual switching, configured weapon assets and sockets, and completed local validation for normal attack weapon display and movement pose switching.

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `5a72375` | (see git log) |
| `5951f55` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 8: 修复 Dodge 冷却卡死

**Date**: 2026-05-25
**Task**: 修复 Dodge 冷却卡死
**Branch**: `master`

### Summary

修复 Dodge 首次成功后 Cooldown.Dodge 不回收的问题，稳定冷却清理路径并完成本地构建与 PIE 跑测通过。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `68a9127` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 9: 收尾普通攻击三段连续性优化

**Date**: 2026-05-27
**Task**: 收尾普通攻击三段连续性优化
**Branch**: `master`

### Summary

完成 05-25-normal-attack-chain-continuity-polish 收尾：补充普攻 Montage Notify 联调规范，归档任务，并保留并行任务改动不纳入本次提交。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `58b9459` | (see git log) |
| `042bfc4` | (see git log) |
| `3598492` | (see git log) |
| `3f8f7e7` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 10: 最小敌对攻击探针收尾与策划配置

**Date**: 2026-05-27
**Task**: 最小敌对攻击探针收尾与策划配置
**Branch**: `master`

### Summary

完成最小敌对攻击探针的策划侧 BP 包装、PIE 白盒日志验证与 AttackFinished 收尾边界修复，归档当前子任务。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `0ee0330` | (see git log) |
| `c00224f` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 11: 玩家受击最小闭环实现与白盒验收

**Date**: 2026-05-27
**Task**: 玩家受击最小闭环实现与白盒验收
**Branch**: `master`

### Summary

完成玩家受击最小结果闭环实现，补齐 HUD 与结构化日志，经过两轮 PIE 白盒验收后修复收尾信号污染问题并归档任务。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `b0c2150` | (see git log) |
| `b9f1f20` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 12: 完成格挡规则基础升级

**Date**: 2026-05-28
**Task**: 完成格挡规则基础升级
**Branch**: `master`

### Summary

完成攻击段格挡规则升级、Guard 判定日志与 DrawDebug 可视化，打通 GuardRewritten 到 GuardBlocked 的正式链路，并通过多轮构建与 PIE 白盒验证。

### Main Changes

- 将 Guard 从命中后最小窗口重写，升级为基于 `AttackContact` 的正式规则判定链路。
- 在攻击元数据中补齐 `bCanBeGuarded`、最大距离、最大高度差、朝向半角等格挡规则承载，并接到 `HostileAttackProbe` 可调参数。
- Guard 成功时正式落账为 `GuardRewriteQueued -> PlayerHitResult(GuardRewritten) -> PlayerDamageResult(GuardBlocked)`，确保不掉血且不进入新的受击链。
- 补齐 `GuardFailedTooEarly`、`GuardFailedTooLate`、`GuardFailedDistance`、`GuardFailedHeight`、`GuardFailedAngle`、`GuardAttackUnguardable`、`GuardRuleSuccess` 等调试日志。
- 增加 Guard `DrawDebug` 扇区/方向/高度可视化，并在 HUD 上显示当前 guard、dist、angle、guard_height 参数。

### Git Commits

| Hash | Message |
|------|---------|
| `7f3bb69` | feat: 完成格挡规则基础升级 |

### Testing

- [OK] 多次完成 `twoheartsEditor Win64 Development` 构建，结果为 `Succeeded`。
- [OK] 多轮 PIE 白盒验证确认成功链路稳定出现：`GuardRuleEvaluate -> GuardRewriteQueued -> GuardRuleSuccess -> PlayerHitResult(GuardRewritten) -> PlayerDamageResult(GuardBlocked)`。
- [OK] 日志确认成功格挡时 `final=0.00`，且 `health_before` 与 `health_after` 保持一致。
- [OK] 日志确认失败分支仍可命中，至少覆盖 `GuardFailedTooLate` 与 `GuardFailedAngle`。

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 13: 攻击元数据基础验收收尾与后续任务拆分

**Date**: 2026-05-28
**Task**: 攻击元数据基础验收收尾与后续任务拆分
**Branch**: `master`

### Summary

完成攻击描述与命中元数据基础链路、重新生成 compile_commands、回写本 task 验收记录，并在归档后同步更新第二章基础战斗父 task 与后续 05-28 子任务拆分。

### Main Changes

(Add details)

### Git Commits

| Hash | Message |
|------|---------|
| `8b88d45` | (see git log) |
| `dabd3fa` | (see git log) |
| `3b34a5b` | (see git log) |
| `3b03a71` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete
