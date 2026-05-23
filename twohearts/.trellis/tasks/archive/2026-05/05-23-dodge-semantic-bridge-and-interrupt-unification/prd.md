# 基础闪避语义桥接与打断统一

## Goal

作为公共战斗语义层第三个资深程序实施子 task，在公共动作上下文底座和普攻桥接完成后，把基础闪避正式接入公共动作上下文，并把“普通攻击能否被 Dodge 打断”的点对点判断升级为统一公共接入口。

## What I already know

* 本 task 只能在以下两个上游 task 完成后开工：
  `05-23-combat-action-context-foundation`
  `05-23-normal-attack-semantic-bridge`
* 当前 `UTwoHeartsGA_Dodge` 已具备正式 Ability 生命周期：
  方向解析；
  Montage 选择；
  `Root Motion` 校验；
  冷却；
  无敌帧；
  动作完成；
  调试事件输出
* 当前 Dodge 打断普通攻击仍然通过 `UTwoHeartsGA_NormalAttackBase::TryInterruptByDodge` 走点对点判断
* 当前主程序要求这一轮把“统一打断接入口”做出来，但不要求一次性扩成完整规则表系统

## Requirements

* 让 Dodge 正式接入公共动作上下文，至少覆盖：
  动作开始；
  关键阶段或生命周期状态；
  逻辑结束或可衔接时机；
  完全结束
* 把“普通攻击可否被 Dodge 打断”的判断升级为公共层统一接入口，不再只依赖普攻私有实现细节
* 当前接入后必须保持现有 Dodge 行为不回归：
  方向解析；
  冷却；
  无敌帧；
  完成通知；
  调试口径
* 当前统一打断接入口应至少服务于“普攻被 Dodge 打断”这一条真实链路，但不强求本轮把所有未来动作都配置化
* 当前桥接后，公共层应能读取到 Dodge 的当前动作状态，而不需要反查 `UTwoHeartsGA_Dodge` 私有字段

## Acceptance Criteria

* [ ] Dodge 激活后会把当前动作正式登记到公共动作上下文
* [ ] Dodge 关键生命周期状态能通过公共层暴露，而不是只停留在私有布尔值与局部日志中
* [ ] “普通攻击可否被 Dodge 打断”已经升级为公共层统一接入口，而不是仅靠普攻私有函数硬编码
* [ ] 现有 Dodge 冷却、无敌帧、方向与完成链路不回归
* [ ] 普攻与 Dodge 之间的打断关系已可通过公共层统一观察与调试

## Out of Scope

* 完整可配置打断规则系统
* 输入评估与完整预输入
* 格挡、受击、伤害的打断规则接入
* 大规模重做 Dodge 资源、Root Motion 或 Notify 体系

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 固定顺序：
  先完成 `05-23-combat-action-context-foundation`
  再完成 `05-23-normal-attack-semantic-bridge`
  然后才能做本 task
  本 task 验收通过后才允许进入 `05-23-combat-input-evaluation-preinput-hook`
* 当前主要代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.*`
  `Source/twohearts/twoheartsCharacter.*`
* 当前实现原则：
  优先把真实 Dodge 生命周期接进公共层；
  再把已有普攻 <-> Dodge 打断链路改成公共接入口；
  不要一上来做面向所有动作的超大打断配置系统
* 若上游普攻桥接仍未稳定，则暂停本 task，先回修上游，而不是在 Dodge 侧加更多特判去兜

