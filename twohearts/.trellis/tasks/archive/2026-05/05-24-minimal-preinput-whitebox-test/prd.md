# 最小预输入正式实施白盒测试

## Goal

作为资深测试工程师，针对已完成归档的 `05-24-minimal-preinput-implementation` 做一轮正式白盒测试，验证“最小预输入”是否真的按 PRD 落到了公共动作上下文、统一输入评估与 Ability 消费链路上，并输出可复现的问题记录、白盒结论与可执行的 PIE 跑测清单。

## What I already know

* 上游实现 task：`archive/2026-05/05-24-minimal-preinput-implementation`
* 上游目标是把最小预输入从 `ReserveForFutureBufferConsumer` 升级为正式的缓冲与消费链路
* 上游要求至少覆盖两条真实链路：
  普攻后续攻击输入；
  Dodge 完成后的后续动作输入衔接
* 当前项目已经存在可观察入口：
  `twoheartsDebugHUD` 中的 action context / buffered input 字段；
  普攻 debug event；
  Dodge debug event；
  PlayerController 里的 debug 控制命令
* 当前角色是资深测试工程师，默认优先产出测试计划、问题记录和回归结论，不直接改业务代码

## Requirements

* 以白盒方式核对最小预输入的承载、写入、消费、回退与调试口径是否符合上游 PRD
* 测试结论必须区分：
  已确认成立的实现；
  已复现的问题；
  暂未证伪但仍需 PIE 跑测补证据的风险点
* 输出中必须包含：
  白盒代码检查结论；
  PIE 手工跑测清单；
  如发现问题，则给出符合测试规范的问题记录
* 白盒检查必须覆盖以下关键问题：
  缓冲输入由谁写入；
  缓冲输入何时允许消费；
  消费失败后是否有安全回退；
  普攻链路与 Dodge 链路是否共用同一套最小预输入口径；
  HUD / 日志是否足以区分立即执行、已缓冲、已消费、被拒绝
* 若实现与上游 PRD 不一致，测试任务应优先明确差异和影响，不擅自补设计规则

## Acceptance Criteria

* [ ] 已建立测试任务并切换为当前活跃 task
* [ ] 已形成一份可执行的白盒测试计划与 PIE 跑测清单
* [ ] 已对白盒代码路径完成首轮核查，覆盖普攻与 Dodge 两条链路
* [ ] 已输出首轮测试结论：通过项 / 风险项 / 问题项
* [ ] 若发现问题，问题记录满足“现象、触发条件、复现步骤、预期、实际、影响范围、代码落点”这些基本字段

## Out of Scope

* 代替上游重新设计最小预输入方案
* 直接修改 Combat / Ability 业务代码
* 越界扩测到格挡、受击、完整技能输入系统
* 将本 task 扩成性能测试、自动化测试或专项工具开发 task

## Technical Notes

* 当前角色判定：资深测试工程师
* 父 task：`05-19-chapter2-basic-combat`
* 被测实现 task：`archive/2026-05/05-24-minimal-preinput-implementation`
* 优先代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`
  `Source/twohearts/twoheartsPlayerController.*`
* 详细测试计划与 PIE 清单见 `info.md`
