# 公共动作上下文最小底座

## Goal

作为公共战斗语义层首个资深程序实施子 task，先落一层最小正式的“公共动作上下文”底座，让普通攻击与基础闪避后续可以共享动作类型、动作阶段、逻辑结束与动作结束原因，而不是继续各自维护私有真相源。

## What I already know

* 当前父 task `05-21-combat-semantic-layer-readiness` 已明确：公共语义层允许开工，但必须按固定顺序推进
* 当前固定顺序第一步就是本 task；未完成本 task，不允许直接进入普攻桥接、Dodge 桥接或输入评估
* 普攻当前已经在 `UTwoHeartsGA_NormalAttackBase` 内维护 `Startup / Active / Recovery / LogicEnded`
* 基础闪避当前已经具备正式 Ability 生命周期、冷却、无敌帧和调试口径，但还没有接入统一动作上下文
* 当前项目技术路线仍是 `UE5 + GAS + C++`，不允许回退到 `AtwoheartsCharacter` 状态机

## Requirements

* 提供一套可长期承载的最小公共动作语义真相源，至少覆盖：
  动作类型；
  动作阶段；
  动作结束原因；
  当前动作上下文快照；
  动作开始 / 阶段切换 / 逻辑结束 / 完全结束的统一出口
* 当前实现必须以“最小可桥接底座”为目标，不提前扩展成完整战斗中台
* 当前底座必须能被普通攻击与 Dodge 后续接入，而不是只为某一个动作特化
* 当前底座应优先落在 `Combat` 层正式 `C++` 代码中，不依赖蓝图临时真相源
* 当前底座需要给后续 task 留出可读接口，但本轮不要求把普攻和 Dodge 完整迁完
* 需要给调试或日志留出最小公共观测口径，便于后续桥接时确认当前动作上下文是否正确写入

## Acceptance Criteria

* [ ] 已存在正式公共定义，至少包含动作类型、动作阶段、动作结束原因和当前动作上下文结构
* [ ] 已存在统一入口，用于开始动作、切换阶段、标记逻辑结束和结束动作
* [ ] 当前底座不依赖 `AtwoheartsCharacter` 私有状态机，也不把核心语义只写在蓝图里
* [ ] 普攻与 Dodge 后续 task 已能基于本 task 产物接入，而不需要再各自重复发明一套上下文结构
* [ ] 已给出最小调试或日志观察口径，能帮助后续 task 验证公共上下文是否真实生效

## Out of Scope

* 普通攻击完整桥接实现
* 基础闪避完整桥接实现
* 输入评估与最小预输入正式实现
* 格挡、受击、伤害、结印接入
* 面向未来所有动作的一次性重抽象或过度可配置化

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 本 task 是固定顺序第 `1` 步；未完成前不允许启动：
  `05-23-normal-attack-semantic-bridge`
  `05-23-dodge-semantic-bridge-and-interrupt-unification`
  `05-23-combat-input-evaluation-preinput-hook`
* 推荐优先阅读代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.*`
  `Source/twohearts/twoheartsCharacter.*`
* 推荐实现方向：
  在 `Source/twohearts/TwoHearts/Combat/` 下新增公共动作上下文相关类型与正式承载类；
  允许命名微调，但职责至少应清晰区分：
  动作上下文数据；
  生命周期更新接口；
  最小公共调试出口
* 本轮优先保真，不优先做大而全架构设计：
  先让真实现有动作有稳定接入口，再考虑未来更多动作的泛化
* 若实现过程中发现当前 `ETwoHeartsCombatPhase` 无法直接复用，可在不破坏现有普攻行为的前提下升级或桥接，但不允许造成“双枚举并存且语义漂移”

