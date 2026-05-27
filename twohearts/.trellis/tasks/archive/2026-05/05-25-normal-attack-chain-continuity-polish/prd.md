# 普通攻击三段连续性优化

## Goal

作为 `05-19-chapter2-basic-combat` 下的一个局部手感收口子 task，明确普通攻击 `1 / 2 / 3` 三段之间的连续性优化需求，让程序能够在现有 `UE5 + GAS + C++` 普攻正式承载之上，收紧 `1 -> 2`、`2 -> 3` 的连段停顿感，避免“前一段明显僵硬收尾后，下一段才开始”的割裂表现。

## What I already know

* 当前已经完成最小预输入、普通攻击和闪避的第一轮接通
* 普通攻击当前为 `1 / 2 / 3` 三段，每一段都是独立 Montage 段
* 实机跑测观察到的主要问题不是“连不上”，而是“连得很硬”：
  当前表现更像 `第一段结束 -> 明显收尾停顿 -> 第二段启动`
* 当前问题属于最小预输入跑通之后的普通攻击局部手感 / 表现收口，不是基础战斗主底座阻断
* 当前仍应服从第二章基础战斗阶段边界，不在这一轮扩散到整套公共战斗规则重做
* 主程序视角补充判断：
  当前输入评估与缓冲链路已经存在，问题更可能出在“下一段真正被激活得太晚”，而不是“输入根本没有收进去”

## Requirements

* 本轮目标是优化普通攻击三段连段的连续性感知，重点处理 `1 -> 2`、`2 -> 3` 的衔接
* 需要把程序需求明确到“下一段何时允许确认”“确认后何时允许起播”“前一段 Recovery 是否允许被连段截断”这三个层面，而不是只停留在“预输入已经存下来了”
* 当玩家在设计允许的连段窗口内输入下一段普通攻击时，下一段应能在前一段已完成主要攻击表达之后、但尚未进入完整僵硬收尾之前衔接出去
* 普通攻击每一段都应该保留可读的启动、命中和基础收招语义；但只要连段输入已成立，前一段过长、过硬的收尾不应继续完整播放到位
* 若玩家没有继续输入，则当前段仍按原本单段攻击逻辑自然收尾，不能因为本轮优化导致单发普通攻击显得被强行截断
* 若输入时机太晚、已超过允许连段窗口，则应保持“不接下一段”的明确结果，不能为了追求顺滑而无限放宽窗口
* 连段优化后仍必须保持 `1 -> 2 -> 3` 的顺序稳定，不允许跳段、吞段或重复触发同一段
* 本轮需求允许程序通过 Montage 段内时机、分段跳转、提前切入下一段、缩短可被连段时的 Recovery 等方式实现，但不预设唯一技术方案
* 主程序明确补充：
  本轮不能只微调“输入可排队窗口”，还必须解决“下一段真正何时开始执行”的问题；
  否则大概率只能缓解，不能根治当前的停顿感
* 如果当前问题经验证主要来自动画资产收尾本身过长，而不是逻辑切段时机错误，程序需要在任务结论中明确指出“当前逻辑已到位，后续还需要动画资产 / Montage 时序二次收口”，不要把问题混写成逻辑已完全解决

## Acceptance Criteria

* [ ] 连续按下普通攻击时，`1 -> 2`、`2 -> 3` 的视觉衔接不再表现为“上一段完整僵硬收尾后，下一段才开始”
* [ ] 在允许连段的输入时机内，下一段普通攻击能够在前一段主要攻击表现完成后尽快进入，而不是等到整段 Montage 自然播完
* [ ] 不继续输入时，单段普通攻击仍能自然收尾，不出现异常抢切、卡断或手感发飘
* [ ] 连续输入普通攻击时，仍严格按照 `1 -> 2 -> 3` 顺序推进，不出现跳过第二段、重复同段或丢段
* [ ] 本轮改动没有越界扩散到闪避、受击、格挡或新的完整 Combo 框架设计
* [ ] 若本轮仍残留明显停顿，任务结果必须能说明剩余问题主要属于逻辑时机、Montage 配置还是动画资源本身

## Technical Approach

### 主程序评估结论

* 当前 PRD 的设计方向是正确的，可以作为解决这次普通攻击不连贯问题的需求基础
* 但要真正解决问题，程序实现必须把“下一段起播时机”从“当前段结束后”前移到“当前段达到可切出点时”
* 如果程序只保留现有“等当前段 Finish 再激活下一段”的模式，这份 PRD 无法完全兑现目标

### 现有代码上下文

* 输入入口在 `AtwoheartsCharacter::HandleAbilityInputPressed`
  普攻输入会先进入 `UTwoHeartsCombatActionContextComponent::EvaluateInputForAction`
* 当前普攻同类输入已经有两条正式口径：
  `Startup / Active` 阶段可走 `ForwardToActiveAbility`
  较晚输入可走 `ReserveForFutureBufferConsumer`
* 当前 `UTwoHeartsGA_NormalAttackBase::InputPressed` 只是把下一段请求记到 `bHasQueuedNextSegment`
* 当前 `UTwoHeartsGA_NormalAttackBase::TryConsumeLateBufferedNextSegment` 能消费较晚缓冲输入
* 但无论是哪一种输入来源，当前真正的下一段激活都还是在 `UTwoHeartsGA_NormalAttackBase::FinishSegment` 之后，通过 `AttemptDeferredNextSegmentActivation` 触发
* 因此现有结构更接近“段尾接段”，而不是“攻击表达完成后立即切段”

### 建议实现方向

* 继续沿用现有 `UE5 + GAS + C++` 正式承载，不回退到 `Character` 级临时状态机
* 连续性优化应优先落在 `UTwoHeartsGA_NormalAttackBase` 内部，而不是改造整套通用输入框架
* 推荐新增“下一段可切出时机”的显式机制，由普攻 Ability 决定何时真正推进到下一段
* 这个切出时机建议优先由以下两类方式之一驱动：
  `Montage Notify`
  或按段配置的归一化 fallback 时间
* 输入缓冲链路继续回答“玩家是否已经在窗口内给出下一段输入”
* 普攻 Ability 新增逻辑回答“当前是否已经到达可顺滑切到下一段的时机”
* 当切出时机到达且下一段输入已成立时，应立即结束当前段并激活下一段，而不是继续等待整段 Section 自然播完
* 当切出时机到达但没有下一段输入时，当前段继续自然播放并按原有方式收尾
* 现有 `FinishSegment` 相关逻辑应继续保留为兜底，以防 Notify 缺失或配置异常导致连段完全失效

### 建议资深程序重点排查的问题

* 连段确认窗口是否过晚
* 下一段起播是否被绑定到前段 Montage 过晚的结束点
* 连段成立后是否仍保留了过长、不可被连段截断的 Recovery
* 当前停顿感主要来自逻辑时机，还是来自动画资产尾巴过长

### 建议程序侧配置与接口落点

* 重点实现文件：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
* 参考输入链路文件：
  `Source/twohearts/twoheartsCharacter.cpp`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
* 角色上的 Montage / Section 配置入口：
  `Source/twohearts/twoheartsCharacter.h`
  `NormalAttackMontage`
  `NormalAttackSectionNames`
* 本轮推荐优先增加的 Ability 内部配置：
  `NextSegmentAdvanceNotifyName`
  `NextSegmentAdvanceFallbackNormalizedTime`
  或其他等价的“连段切出点”配置
* 如果三段都共用同一 Montage，也允许先采用统一 Notify 名，只通过各段 Notify 摆放位置区分时机
* 若后续发现每段节奏差异明显，再扩展成按段独立配置；这一轮不要求提前抽成完整 Combo 框架

## Out of Scope

* 闪避、受击、格挡等其他基础战斗动作的连续性改造
* 为普通攻击重做完整公共 Combo 系统
* 新增完整数值体系、伤害判定体系或命中反馈体系
* 为未来所有动作统一设计一套更大的输入规则表
* 直接要求重做全部普通攻击动画资源

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 当前任务定位：最小预输入跑通后的普通攻击局部手感收口
* 上游已知能力：
  `05-24-minimal-preinput-implementation`
  已经让“输入可被存下并在后续条件满足时消费”成为正式链路的一部分
* 当前关键代码观察：
  `AtwoheartsCharacter::HandleAbilityInputPressed`
  `UTwoHeartsCombatActionContextComponent::EvaluateInputForAction`
  `UTwoHeartsGA_NormalAttackBase::InputPressed`
  `UTwoHeartsGA_NormalAttackBase::FinishSegment`
  `UTwoHeartsGA_NormalAttackBase::AttemptDeferredNextSegmentActivation`
* 给程序的需求口径应优先强调设计结果，但这次必须补充到足以指导资深程序定位代码问题
* 本轮如需额外白盒观察，建议至少覆盖：
  单按一段；
  稍快节奏连按 `1 -> 2 -> 3`；
  接近窗口末尾的延后输入；
  明确不该接上的过晚输入
* 对应旧文档：`../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
