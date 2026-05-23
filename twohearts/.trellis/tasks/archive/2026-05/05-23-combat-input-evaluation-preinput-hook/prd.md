# 战斗输入评估与最小预输入接入口

## Goal

作为公共战斗语义层第四个资深程序实施子 task，在公共动作上下文、普攻桥接和 Dodge 桥接都稳定后，统一战斗输入评估结果，并为下一阶段“最小预输入”提供正式接入口，但本轮不直接完成完整预输入功能。

## What I already know

* 本 task 必须排在四个 task 的最后一个，只有前三个完成后才允许开工
* 当前最小预输入之所以不能提前做，是因为它依赖：
  当前动作是什么；
  当前动作处于哪个阶段；
  当前动作是否允许被打断；
  当前动作逻辑何时结束
* 当前这些信息在本 task 开工前，应当已经由公共动作上下文、普攻桥接和 Dodge 桥接提供稳定真相源
* 当前主程序要求本轮统一的是输入评估结果和接入口，而不是直接把完整缓存执行机制一次性做完

## Requirements

* 提供统一输入评估入口，至少能输出：
  `ExecuteNow`
  `BufferInput`
  `Reject`
* 输入评估必须优先依赖公共动作上下文与统一打断口径，不再分散读取各动作私有状态
* 当前实现需要覆盖至少两条真实链路：
  普攻输入评估；
  Dodge 输入评估
* 当前实现需要给下一阶段“最小预输入”留下稳定接入口，但不要求本轮真正执行完整缓存消费
* 当前实现不得回退成“在 Character 和各 Ability 里各写一套输入判断”
* 需要保留或补充最小调试口径，便于观察一次输入被判为立即执行、缓存还是拒绝的原因

## Acceptance Criteria

* [ ] 已存在统一输入评估入口，并能返回 `ExecuteNow / BufferInput / Reject`
* [ ] 普攻与 Dodge 至少已能接入同一套输入评估口径，而不是继续走完全分散的私有判断
* [ ] 当前输入评估会读取公共动作上下文、当前阶段与统一打断口径，而不是绕开公共层
* [ ] 当前实现已为下一阶段最小预输入提供稳定正式接入口
* [ ] 本轮没有越界把完整预输入缓存执行、格挡输入或受击输入一并做掉

## Out of Scope

* 完整预输入缓存执行
* 格挡、受击、技能系统输入统一
* 面向未来所有动作的全量配置化输入规则表
* 新一轮大规模输入资产或蓝图重构

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 固定顺序：
  本 task 只能在以下 3 个 task 验收通过后开工：
  `05-23-combat-action-context-foundation`
  `05-23-normal-attack-semantic-bridge`
  `05-23-dodge-semantic-bridge-and-interrupt-unification`
* 当前主要代码落点：
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  以及前三个 task 新增的公共动作上下文相关正式承载代码
* 当前实现原则：
  先统一“输入评估结果”与“输入评估入口”；
  缓存如何消费、何时正式执行，留给后续“最小预输入” task 做
* 若前三个 task 产出的公共动作上下文或统一打断口径仍不稳定，应优先回修上游，不在本 task 内额外发明补丁逻辑

