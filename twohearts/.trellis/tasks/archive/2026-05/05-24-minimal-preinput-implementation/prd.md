# 最小预输入正式实施

## Goal

作为第二章基础战斗模块下、公共战斗语义层之后的下一直接开工子 task，由资深程序在现有统一输入评估与公共动作上下文之上，正式落地“最小预输入”第一轮可用版本，让普攻与 Dodge 的后续输入不再只有评估结果和预留口子，而是具备最小可消费的正式缓冲执行链路。

## What I already know

* `05-21-combat-semantic-layer-readiness` 已归档完成
* 当前已经存在统一输入评估口径：
  `ExecuteNow`
  `BufferInput`
  `Reject`
* 当前已经存在最小预输入接入口，但晚到输入仍可能只落在：
  `ReserveForFutureBufferConsumer`
  也就是“预留给未来消费者”，还没有正式消费机制
* 当前公共动作上下文已经正式承载：
  动作类型；
  动作阶段；
  逻辑结束；
  动作结束原因；
  普攻 <-> Dodge 的真实打断链路
* 当前第二章主线顺序已经推进到：
  最小预输入
  -> 受击与伤害
  -> 格挡

## Requirements

* 在现有公共动作上下文和统一输入评估之上，补齐“最小预输入”的正式消费机制
* 本轮至少覆盖两条真实链路：
  普攻后续攻击输入；
  Dodge 完成后的后续动作输入衔接
* 当前实现必须把“缓冲输入存下来”“何时允许消费”“由谁消费”三件事明确落到正式代码，而不是继续只停留在日志口径
* 当前实现必须优先复用已有公共动作上下文、输入评估和 Ability 承载，不允许回退成 Character 私有临时状态机
* 当前实现必须保留清晰的调试口径，便于白盒验证一次输入是被立即执行、已缓冲待消费、成功消费还是被拒绝
* 当前实现应以“最小可用”作为目标，不提前扩成完整技能输入系统或全动作配置化缓冲系统

## Acceptance Criteria

* [ ] 当前已存在正式的最小预输入承载与消费链路，而不是只有 `ReserveForFutureBufferConsumer`
* [ ] 普攻至少有一条真实“输入缓冲 -> 条件满足 -> 自动消费”的链路可跑通
* [ ] Dodge 完成后的后续动作输入衔接已能接入同一套最小预输入口径，或明确说明本轮为何只先覆盖普攻主链路
* [ ] 当前最小预输入仍建立在公共动作上下文与统一输入评估之上，没有回退到 Character 级分散判断
* [ ] HUD / 日志 / 调试输出能够区分：
  立即执行；
  已缓冲；
  已消费；
  被拒绝
* [ ] 本轮没有越界把格挡、受击、完整技能输入或大而全配置系统一并做掉

## Out of Scope

* 格挡正式实现
* 受击与伤害系统正式实现
* 面向未来所有动作的完整输入规则表与重型配置系统
* 技能系统、敌人系统、武器系统输入统一接入
* 为了本 task 再创建“拆分 task / 设计 task / 管理 task”的中间 task

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 当前上游已完成阶段：
  `05-21-combat-semantic-layer-readiness`
* 主要参考代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
* 推荐优先实现顺序：
  1. 明确最小预输入承载结构与生命周期
  2. 先打通普攻主链路的缓冲与消费
  3. 再确认 Dodge 后续输入衔接
  4. 最后补调试输出与白盒观察口径
* 对应旧文档：`../docs/a双心印开发进度简介.md`
