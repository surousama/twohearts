# 攻击描述与命中元数据基础实施说明

## 当前结论

1. 当前适合继续在现有实现上叠加功能，不需要回退重做 `hostile probe`、`player hit result` 或基础 `Guard`。
2. 本 task 有明确顺序要求：它是 `05-28-player-damage-result-formalization`、`05-28-player-hit-reaction-minimum-implementation`、`05-28-guard-rule-foundation-upgrade` 的硬前置；`05-28-guard-outcome-settlement-and-counter-hook` 与 `05-28-preinput-second-pass-generalization` 不建议提前。
3. 当前没有阻断本阶段的严重漏洞；现有 `FTwoHeartsHostileAttackSignal` 与 `FTwoHeartsPlayerHitResult` 属于当前阶段可接受的过渡承载，但它们仍偏“来袭时序 / 玩家结果”，还不是后续多系统共享的正式攻击语义真相源。

## 当前目标

1. 建立一份正式、最小、共享的“攻击描述 / 命中元数据”结构，作为后续伤害、受击、格挡的共同输入。
2. 让 `ATwoHeartsHostileAttackProbeCharacter` 在每次攻击实例上稳定产出该结构，而不是继续只发零散字段。
3. 让 `UTwoHeartsHostileAttackReceiverComponent` 在收信号与生成玩家结果时保留并暴露该结构，避免后续 task 再从 `SignalType`、`Detail` 字符串和布尔值反推攻击语义。
4. 给 `UTwoHeartsGA_NormalAttackBase` 预留同结构的段级挂载点或默认构造口，让玩家普攻未来也能走同一条输入语义链路。

## 本轮明确不做

1. 不做正式 HP / 扣血 / 死亡。
2. 不做正式受击状态、硬直、击退或受击动画。
3. 不做完整敌人攻击框架重构。
4. 不做完整战斗 DataAsset 平台。
5. 不把 `Guard` 成功 / 失败规则、位移结果、伤害结果一次性并入本 task。

## 当前代码现状判断

### 1. 现有 hostile signal 已经具备“实例 + 来源 + 朝向 + 命中窗口”的最小闭环

1. `FTwoHeartsHostileAttackSignal` 目前已有：
   `AttackInstanceName`、`SourceActor`、`TargetActor`、`SourceLocation`、`AttackDirection`、`TimestampSeconds`、`bIsHitWindowActive`、`bHasContact`、`Detail`。
2. 这足够支撑当前最小 probe -> player result -> guard rewrite 闭环。
3. 但它还缺：
   受击反应类型；
   伤害 / 机制标签；
   是否可格挡；
   是否可被闪避规避；
   更稳定的最小时序语义字段。

### 2. 现有 player hit result 已经是“玩家侧结果”，不应继续承担“攻击描述真相源”职责

1. `FTwoHeartsPlayerHitResult` 当前重点在：
   `ResultType`、`bHitConfirmed`、`bCanBeRewrittenByGuard`、`SourceSignalType`、`ResultTimestampSeconds`。
2. 它的职责应该继续偏“玩家这次结果如何”，而不是把所有攻击规则塞进去。
3. 若把正式攻击语义继续硬塞进 `PlayerHitResult`，后续普通攻击、敌方攻击、格挡规则就会再次耦合到“玩家结果结构”，不利于复用。

### 3. 当前最佳切入点是“signal 之前先造共享元数据，再让 signal / result 都引用或拷贝它”

1. 最合理的顺序不是先做伤害或受击，而是先补共享输入层。
2. 共享元数据应先由 hostile probe 产出，再由 receiver 保留，后续伤害 / 受击 / guard 统一读取。
3. 普攻本轮允许只补挂载口，不强行实现敌方受击消费侧。

## 建议结构方案

### 1. 共享数据结构

建议先抽到中性位置，例如：
`Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`

当前建议至少包含：
1. 攻击实例名或实例标识。
2. 攻击来源 actor。
3. 最小受击方向基准：
   来源位置；或攻击方向；或两者同时保留。
4. 受击反应类型：
   本轮可先用单值 enum；
   或单值 `Gameplay Tag`；
   关键是和“伤害 / 机制标签”分层，不要混在一个字段里。
5. 伤害 / 机制标签：
   建议直接使用 `FGameplayTagContainer`。
6. `bCanBeGuarded`。
7. `bCanBeDodged`。
8. 最小时序语义：
   本轮允许只做到“攻击实例 + 当前命中阶段 / 窗口标识”，
   不要求一次性抽完整时间轴系统。

### 2. hostile probe 产出层

建议修改：
1. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.h`
2. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.cpp`

建议做法：
1. 每次 `StartAttackStartup()` 时构造当前攻击实例对应的共享元数据。
2. 后续 `AttackStarted / HitWindowOpened / AttackContact / HitWindowClosed / AttackFinished` 都沿用这份元数据，而不是每次零散重组。
3. 本轮允许 hostile probe 的默认元数据先写死为最小样本，例如：
   反应类型 = 轻攻击；
   机制标签 = 物理攻击；
   可格挡 = true；
   可闪避规避 = true。
4. 但这些默认值必须落在结构化字段里，不能只存在于 `Detail` 文本中。

### 3. receiver 桥接层

建议修改：
1. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
2. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`

建议做法：
1. `FTwoHeartsHostileAttackSignal` 应直接携带共享元数据，或至少稳定引用 / 拷贝它。
2. `FTwoHeartsPlayerHitResult` 可保留一份“本次结果对应的攻击元数据快照”或等价字段，但不要反客为主。
3. Guard 当前改写链路不能断：
   `HitConfirmed -> GuardRewritten` 仍应成立；
   只是后续改写不再只基于裸布尔值，而是能读到正式攻击规则输入。

### 4. normal attack 挂载层

建议修改：
1. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
3. 必要时再落到 `TwoHeartsGA_NormalAttack_1/2/3.*`

建议做法：
1. 本轮不强制把普攻接成完整敌方受击链路。
2. 但应至少给段级攻击元数据一个正式挂载口：
   可在 `NormalAttackBase` 提供默认字段；
   也可让不同段覆盖默认值。
3. 这样下一步做正式伤害 / 敌方消费时，不会又重新发明一套“普攻攻击描述”。

### 5. tags 与调试层

可能涉及：
1. `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
2. `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`
3. `Source/twohearts/twoheartsDebugHUD.cpp`

建议做法：
1. 若本轮使用 `Gameplay Tag` 表达“伤害 / 机制标签”，应补最小 tags，但不要一次性铺很大标签树。
2. HUD 或日志至少应能看到：
   受击反应类型；
   机制标签；
   是否可格挡；
   是否可闪避规避；
   当前实例名。
3. 调试可读性要继续保留，因为后续 Guard 二期和伤害正式化会直接依赖这些观察口径。

## 施工注意事项

1. 不要把共享攻击语义继续散落在 `Detail` 字符串、`SignalType` 分支和零散布尔值中。
2. 不要把 `AtwoheartsCharacter` 重新拉回长期战斗状态承载层；正式真相源仍应留在 `Combat` 结构与相关组件 / Ability。
3. 不要为了未来一次性做完整敌人配置平台；本轮只需要保证字段与接口不再阻断后续阶段。
4. 受击反应类型与伤害 / 机制标签必须拆层表达；即使本轮实现很轻，也不能混成一个字段。
5. 若本轮使用默认硬编码样本值，必须明确标注为“当前阶段默认值”，避免被误认为完整数据驱动体系。

## 风险点

1. 若把共享结构继续放在 `HostileAttackReceiverComponent` 私有内部，后续普通攻击无法自然复用。
2. 若本轮只给 hostile probe 补结构，却不给 normal attack 留挂载口，下一阶段做玩家普攻伤害时仍会二次造轮子。
3. 若本轮过早上完整标签平台或完整 DataAsset，会明显超出当前阶段边界。
4. 若 HUD / 日志不补最小可见字段，后续 Guard 二期很容易再次陷入“逻辑可能对了，但看不到到底拿了什么规则输入”的联调问题。

## 建议验收口径

1. hostile probe 每次攻击实例都能稳定产出结构化攻击元数据。
2. `FTwoHeartsHostileAttackSignal` 不再只是“时序 + 布尔”，而是携带正式攻击语义。
3. `UTwoHeartsHostileAttackReceiverComponent` 对外能拿到与本次玩家结果对应的攻击元数据。
4. 当前 Guard 改写链路不回退、不失效。
5. `UTwoHeartsGA_NormalAttackBase` 已具备同结构挂载口或默认构造口。
6. 本轮没有越界扩散到正式伤害、正式受击状态或完整格挡结算。

## 开工后的第一优先级建议

1. 先确定共享结构放在哪里，以及字段最小集合。
2. 再改 hostile probe 的产出链路。
3. 再改 receiver 的桥接与暴露方式。
4. 最后补 normal attack 的挂载口与 HUD / 日志调试面。

## 对下游 task 的直接价值

1. `05-28-player-damage-result-formalization` 可以直接读取正式攻击语义，而不是再从 `HitConfirmed` 反推“这是什么攻击”。
2. `05-28-player-hit-reaction-minimum-implementation` 可以直接读取受击反应类型与方向基准。
3. `05-28-guard-rule-foundation-upgrade` 可以直接读取“是否可格挡”“是否可闪避规避”“最小时序语义”，不必重建输入层。

## 2026-05-28 实际实现回写

### 已完成内容

1. 新增共享结构 `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`，正式承载：
   `AttackInstanceName`、`SourceActor`、`SourceLocation`、`AttackDirection`、`HitReactionType`、`DamageMechanicTags`、`bCanBeGuarded`、`bCanBeDodged`、`TimingPhase`、`TimingWindowName`。
2. `ATwoHeartsHostileAttackProbeCharacter` 已在每次攻击实例开始时构造当前攻击元数据，并在 `Startup / HitWindow / Recovery / Finished` 阶段持续更新时序字段。
3. `FTwoHeartsHostileAttackSignal` 已直接携带 `AttackMetadata`；`UTwoHeartsHostileAttackReceiverComponent` 收到信号后会把该结构继续保留到 `FTwoHeartsPlayerHitResult` 中。
4. 当前 `Guard` 改写链路保持原样未断，但 `HitConfirmed` 是否仍可被 Guard 改写，已经开始由 `AttackMetadata.bCanBeGuarded` 驱动，而不是永远硬编码为 `true`。
5. `UTwoHeartsGA_NormalAttackBase` 已补上同结构的正式挂载口 `AttackMetadataTemplate` 与运行时构造函数 `BuildCurrentAttackMetadata()`，便于后续把玩家普攻正式接入敌方消费侧。
6. `TwoHeartsGameplayTags` 已补最小机制标签：
   `Attack.Mechanic.Physical`、`Attack.Mechanic.HostileProbe`、`Attack.Mechanic.NormalAttack`。
7. `twoheartsDebugHUD.cpp` 与相关日志已补最小观察口径，可直接看到：
   受击反应类型、机制标签、是否可格挡、是否可闪避规避、当前时序标识。

### 当前阶段默认值说明

1. hostile probe 当前默认攻击语义仍是最小样本：
   受击反应类型 = `Light`；
   机制标签 = `Physical + HostileProbe`；
   可格挡 = `true`；
   可闪避规避 = `true`。
2. normal attack 当前默认挂载值也是最小样本：
   受击反应类型 = `Light`；
   机制标签 = `Physical + NormalAttack`；
   其作用是提供统一挂载口，不代表本轮已经打通正式敌方受击链路。

### 已知限制

1. 本轮仍未实现正式玩家受击 `Gameplay Ability`；这里只先把其未来读取的攻击语义真相源搭好。
2. `FTwoHeartsPlayerHitResult` 仍保留旧阶段兼容字段（如 `AttackInstanceName`、`SourceActor`、`bCanBeRewrittenByGuard`），后续 task 应逐步把判断重心迁到 `AttackMetadata`。
3. 当前 normal attack 只提供元数据挂载与构造口，尚未把玩家普攻实际送入敌方命中消费链。

### 本轮检查

1. 已对新增/修改文件读取 IDE 诊断，当前无新增 lints。
2. `twoheartsEditor Win64 Development` 已重新编译通过。
3. 未扩散到正式伤害、正式受击状态或完整格挡结算。

### 2026-05-28 PIE 白盒验收结果

1. 已覆盖 `HitConfirmed` 路径：`HostileProbe_1`、`HostileProbe_3` 在命中窗口内成功触发 `AttackContact -> HitConfirmed`，并把完整 `AttackMetadata` 带入 `FTwoHeartsPlayerHitResult`。
2. 已覆盖 `GuardRewritten` 路径：`HostileProbe_2`、`HostileProbe_4` 在基础 Guard 生效窗口内被改写为 `GuardRewritten`，且改写后仍保留同一份攻击元数据快照。
3. 已覆盖 `HitExpired` 路径：后续补测中 `HostileProbe_1`、`HostileProbe_2`、`HostileProbe_3` 均表现为 `HitWindowOpened -> ContactMiss -> HitExpired -> AttackFinished`，说明未命中/过期分支已稳定成立。
4. 三条运行时关键路径 `HitConfirmed / GuardRewritten / HitExpired` 均已在 PIE 日志中实测覆盖；`AttackMetadata` 的 `TimingPhase / TimingWindowName` 也已验证会沿 `Startup -> HitWindow -> Recovery -> Finished` 推进。

### 收尾判断

1. 本 task 的验收口径已满足，当前可视为完成实施与验证。
2. 本轮没有形成足够稳定、足够通用、需要写回 `.trellis/spec/` 的跨任务新规则；相关实现经验与阶段性默认值已沉淀在当前 `info.md`，后续若 Guard 二期或正式伤害阶段再次抽象出稳定模式，再统一回写 spec。
3. 为提升后续白盒跑测效率，已把 `HostileAttackProbe / PlayerHitResult / Guard` 之外的大量过程态日志降到 `Verbose`；当前 `Display` 层日志已足够支撑验收判断。
