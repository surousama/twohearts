# 预输入二期泛化

## Goal

作为 `05-19-chapter2-basic-combat` 之下、位于受击 / 伤害正式化与 Guard 二期之后的执行子 task，把当前“最小预输入”升级成更完整、可配置、可清理的统一预输入系统，使普通攻击、Dodge、Guard 以及后续技能输入不再只共享最小缓冲链路，而是共享更稳定的通用预输入规则。

## What I already know

* `05-24-minimal-preinput-implementation` 与 `05-24-minimal-preinput-whitebox-test` 已经完成最小预输入首轮闭环。
* 当前最小预输入已经足够证明“缓冲 -> 消费”主链路存在，但它还不是原始框架文档要求的完整预输入系统。
* 当前仍缺：
  只保留最后一个合法输入；
  配置化窗口；
  被受击或强制中断时统一清空；
  消费失败后的稳定策略；
  更完整动作集合纳入统一规则。
* 因此，本 task 不应早于受击 / 伤害与 Guard 二期，因为它需要建立在更完整动作集合和更稳定规则层之上。

## Requirements

* 在现有统一输入评估与最小缓冲链路之上，把预输入升级成更完整的统一系统。
* 当前至少要补齐：
  只保留最后一个合法输入；
  角色默认窗口 + 动作覆写的最小配置化窗口；
  被受击 / 强制中断 / 更高优先级行为打断时的统一清空规则；
  消费失败后的最小回退或保留策略；
  普攻、Dodge、Guard 的统一纳入。
* 需要继续沿用公共动作上下文与统一输入评估，不允许回退到 Character 级分散私有判断。
* 需要保留清晰调试可读性，便于说明：
  这次输入是立即执行、进入缓冲、被覆盖、被清空、消费成功还是消费失败。
* 若当前阶段还未接入技能系统，本轮允许先不覆盖完整技能集合，但必须把系统设计成后续技能可接入，而不是再次写死为“只支持普攻和 Dodge”。
* 本轮不应把任务扩大成完整输入配置表平台、完整技能系统接入或所有未来动作一次性统一。

## Acceptance Criteria

* [ ] 当前已经存在“只保留最后一个合法输入”的正式规则承载。
* [ ] 当前已经存在配置化窗口或等价正式承载，而不是只能依赖固定硬编码时间窗。
* [ ] 受击 / 强制中断 / 更高优先级行为时的预输入清空规则已经正式存在。
* [ ] 普攻、Dodge、Guard 至少已纳入同一套二期预输入规则。
* [ ] 当前可以从日志、HUD 或调试字段清楚观察到预输入覆盖、清空、消费成功、消费失败等结果。
* [ ] 本轮没有越界扩散成完整输入平台、完整技能系统或所有未来动作一次性统一。

## Out of Scope

* 完整技能系统输入统一接入
* 完整输入配置工具或编辑器面板
* 所有未来战斗动作一次性纳入
* 与网络同步相关的输入复制问题
* 为预输入二期重做整个动作系统

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-24-minimal-preinput-implementation`
  `05-28-player-hit-reaction-minimum-implementation`
  `05-28-guard-outcome-settlement-and-counter-hook`
* 当前建议优先查看的代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
* 当前允许的过渡边界：
  允许先覆盖普通攻击、Dodge、Guard 三类基础动作；
  允许技能系统留作后续接入；
  但不允许继续把二期预输入做成“最小预输入的一堆特判补丁”。
