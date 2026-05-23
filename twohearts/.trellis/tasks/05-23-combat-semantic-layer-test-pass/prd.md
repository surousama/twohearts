# 公共战斗语义层白盒回归与PIE跑测

## Goal

作为公共战斗语义层首轮代码完成后的测试子 task，围绕公共动作上下文、普攻桥接、Dodge 桥接、统一打断入口和输入评估三态输出，组织一轮可回归的白盒验证与 Unreal Editor PIE 跑测，给出“已复现 / 本轮暂未复现 / 已解决 / 需继续观察”的测试结论。

## What I already know

* 当前主范围包括 4 个已完成或已落代码的实施子 task：
  `05-23-combat-action-context-foundation`
  `05-23-normal-attack-semantic-bridge`
  `05-23-dodge-semantic-bridge-and-interrupt-unification`
  `05-23-combat-input-evaluation-preinput-hook`
* 当前白盒观察入口已经存在：
  公共动作上下文日志
  普攻调试事件
  Dodge 调试事件
  Combat Input Evaluation HUD / 日志
* 当前主程序判断是：
  首轮代码目标基本完成，但尚未完成总体回归验证

## Requirements

* 输出一轮简洁、可复用的测试计划与问题记录口径
* 白盒测试工程需要重点验证以下内容：
  公共动作上下文 `BeginAction -> Transition -> LogicEnded -> FinishAction` 顺序是否稳定
  普攻 `Startup / Active / Recovery / LogicEnded` 是否与公共上下文、HUD、日志一致
  Dodge `Startup / Active / Recovery / LogicEnded` 是否与公共上下文、HUD、日志一致
  Dodge 打断普攻时，统一打断入口是否只在允许阶段放行
  输入评估三态 `ExecuteNow / BufferInput / Reject` 是否与实际行为一致
  `BufferInput` 是“转发给 active ability”还是“留给后续消费者”，是否能从日志/HUD中区分
* 策划在游戏内 PIE 跑测需要重点覆盖以下内容：
  原地与移动中普攻 1-2-3 连段
  普攻 `Startup / Active / Recovery / LogicEnded` 各阶段再次按攻击键的表现
  普攻不同阶段尝试闪避，验证哪些阶段允许打断、哪些阶段应被拒绝
  原地与八方向 Dodge 的方向解析、Root Motion、无敌窗口和冷却表现
  Dodge 完成后能否平滑回到后续动作输入
  输入被拒绝时，是否符合预期而不是“无反馈卡死”
* 测试输出必须遵守白盒问题单规范：现象、触发条件、复现步骤、预期、实际、影响、代码落点、建议修法、复测重点

## Acceptance Criteria

* [ ] 已形成可执行的白盒测试清单
* [ ] 已形成给策划使用的 PIE 跑测关注点清单
* [ ] 每个发现的问题都能按规范落成可复现、可修复、可回归的问题记录
* [ ] 测试结论能区分：
  已复现
  本轮暂未复现
  已解决
  需继续观察
* [ ] 最终结论能支撑父 task 判断“可收口 / 需继续整改”

## Out of Scope

* 直接修改业务代码
* 擅自补设计规则
* 把测试文档写成大而全背景报告
* 扩展到 Guard、受击、伤害与完整技能输入系统

## Technical Notes

* 参考规范：
  `.trellis/spec/testing/whitebox-testing-guidelines.md`
  `.trellis/spec/testing/test-document-rules.md`
* 重点观察入口：
  `ATwoheartsDebugHUD::DrawHUD`
  `UTwoHeartsCombatActionContextComponent::BuildCurrentContextDebugString`
  `AtwoheartsCharacter::GetCombatInputDebugEvents`
  普攻 / Dodge / CombatInputEval 结构化日志
* 当前测试目标不是证明“未来完整预输入已做好”，而是验证当前首轮公共语义层是否稳定、是否具备进入下一轮的条件
