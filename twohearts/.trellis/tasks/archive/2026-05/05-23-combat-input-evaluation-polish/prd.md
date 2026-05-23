# 战斗输入评估收口与消费边界整理

## Goal

作为公共战斗语义层收口后的资深程序整改子 task，在不推翻现有 `ExecuteNow / BufferInput / Reject` 首轮能力的前提下，整理统一输入评估与实际消费逻辑之间的边界，避免后续“最小预输入”继续把规则堆回 `Character`。

## What I already know

* 当前统一输入评估入口已经存在：
  `UTwoHeartsCombatActionContextComponent::EvaluateInputForAction`
* 当前实际输入消费主要落在：
  `AtwoheartsCharacter::HandleAbilityInputPressed`
* 目前 `HandleAbilityInputPressed` 同时承担评估、转发、激活扫描和 debug 记录，函数职责偏重
* 目前公共输入评估已经开始包含普通攻击特有的缓冲窗口逻辑，这一轮可接受，但继续叠加会让公共层变成特例规则集
* 当前目标不是实现完整预输入，而是为下一轮最小预输入留出更稳定的正式接口

## Requirements

* 保持当前对外结果口径不变：
  `ExecuteNow`
  `BufferInput`
  `Reject`
* 在现有架构内整理“公共评估结果”和“实际消费执行”之间的责任边界，降低 `AtwoheartsCharacter::HandleAbilityInputPressed` 的耦合度
* 不把完整预输入缓存消费机制提前塞进本 task
* 保持当前普通攻击连段、Dodge 打断与输入拒绝行为不发生功能性回退
* 让“BufferInput 但当前无人消费”的路径在代码与调试输出中更明确，便于后续最小预输入 task 接入
* 若需要抽辅助函数、局部承载结构或更清晰的接口，应优先轻量收口，不另起一套重型系统

## Acceptance Criteria

* [ ] 统一输入评估与实际消费执行的职责边界比当前更清晰
* [ ] `HandleAbilityInputPressed` 的职责得到收束，不再继续膨胀为后续所有输入规则总入口
* [ ] 当前普通攻击、Dodge、打断与拒绝行为未回退
* [ ] `BufferInput` 的两类情况可明确区分：
  转发给当前 active ability
  留给后续正式 preinput 消费者
* [ ] 调试日志或 HUD 仍能支撑白盒定位，不因整理而丢失观察口径

## Out of Scope

* 完整预输入缓存执行
* Guard / 受击 / 技能输入统一接入
* 大规模重写公共动作上下文系统
* 修改设计文档或主程序评估文档

## Technical Notes

* 上游背景：
  `05-21-combat-semantic-layer-readiness`
  `05-23-combat-semantic-layer-regression-review`
* 重点代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`
* 当前重点风险位置：
  `Source/twohearts/twoheartsCharacter.cpp`
  `HandleAbilityInputPressed`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
  `EvaluateInputForAction`
