# 文档用途

1. 本文档是《双心印》第二章基础战斗模块的主开发文档。
2. 它同时承担三类职责：
   记录当前做到哪一步；
   指明下一步该做什么、依赖什么接口；
   留存已经完成的实现结果、踩坑记录和仍在使用的技术口径。
3. 后续第二章程序推进，默认优先读取本文件，不再把“开发文档”和“归档文档”拆成多份并行维护。

# 使用方式

1. 当前是主程序视角。
2. 主程序每次拿到本文件时，按以下顺序使用：
   先看“当前阶段结论”，判断当前是否适合继续在现有实现上叠功能；
   再看“当前开发顺序”和“下一直接开工项”，决定下一步方向；
   再看“关键接口现状”“技术债与风险”，判断是否需要先补底座；
   最后把本轮要做的事项整理成可直接派给资深程序的开发单。
3. 资深程序每次拿到本文件时，按以下顺序使用：
   先看“下一直接开工项”和对应“实施要求”；
   再看“当前有效技术结构”和“关键接口现状”，确认应接在哪一层做；
   开发完成后，把“本次实际完成记录、代码落点、已知限制、联调结果”回写到本文件；
   如果某项未来开发已明显进入新阶段，再由主程序基于更新后的本文件继续 review 和拆单。
4. 默认开发循环：
   主程序读本文件 -> 判断下一阶段 -> 拆单派活 -> 资深程序按文档开发 -> 资深程序回写实现记录 -> 主程序再次 review 本文件并派下一轮活。
5. 如果后续出现新的阶段性技术文档，默认只作为“当前直接开工子文档”短期使用；
   阶段结束后，必须把有效结论回收进本文件，避免目录再次膨胀。

# 当前阶段结论

1. 第二章基础战斗模块已经完成第一轮正式战斗底座搭建。
2. 当前普通攻击正式承载已稳定落在 `UE5 + GAS + C++` 路线下。
3. `AtwoheartsCharacter` 当前只应继续承担 `ASC` 持有与初始化、输入转发、角色侧资源配置和调试承载。
4. 普通攻击 `1 / 2 / 3` 已形成单路径 `Gameplay Ability` 承载，不应再回退到旧 Character 普攻状态机。
5. 普攻阶段语义 `Startup / Active / Recovery / LogicEnded` 已建立，当前最小 Dodge 打断规则已接通。
6. 当前没有阻断本阶段已完成功能的严重漏洞。
7. 基础闪避正式落地的 `C++` 主体已接通，当前 `UTwoHeartsGA_Dodge` 已具备方向判定、位移执行、冷却、无敌帧状态出口和普攻阶段打断衔接。
8. 当前基础闪避还需要角色蓝图侧补齐 `Dodge Montage` 与参数配置，并完成本地联调验收，才能视为本阶段完全收口。
9. 在基础闪避资源与联调口径稳定前，暂不建议直接切到公共战斗语义层。

# 当前开发顺序

1. 已完成：最小普通攻击闭环
   作用：
   验证 `1-2-3` 普攻闭环可跑通。
2. 已完成：`AbilitySystem` 基础底座
   作用：
   建立正式 `GAS` 接入、Ability 授予链路、基础 Ability 基类和 Tag 出口。
3. 已完成：普通攻击完全切换到 `AbilitySystem`
   作用：
   结束“输入走 GAS、逻辑仍在 Character”的过渡状态。
4. 已完成：普通攻击阶段标记与基础打断
   作用：
   建立普攻阶段语义，并验证最小 Dodge 打断。
5. 已完成：基础闪避正式落地 `C++` 第一版
   作用：
   把当前“只够做打断验证的最小 Dodge”补成具备正式生命周期的基础闪避 Ability。
6. 下一直接开工项：基础闪避资源配置与本地联调
   作用：
   补齐 `Dodge Montage`、角色参数配置和场景内验收，确认方向、位移、冷却、无敌帧和动作衔接都符合口径。
   直接实施文档：
   [a基础闪避正式落地技术文档](./a基础闪避正式落地技术文档.md)
7. 基础闪避验收通过之后：公共战斗语义层
   作用：
   基于“普攻 + 闪避”两个已落地动作，统一收束动作上下文、打断规则、输入评估和逻辑结束事件。
8. 公共语义层之后：最小预输入
   作用：
   基于统一动作上下文和逻辑结束事件实现更稳定的输入缓存，不在普攻和闪避里各写一套。
9. 再往后：
   受击与伤害
   格挡
   更完整的战斗公共规则收束

# 当前主程序评估结论

1. 当前不建议把公共语义层提前到基础闪避之前。
2. 原因一：
   现在虽然已经有“普攻阶段语义 + 最小 Dodge 打断”，但 Dodge 还只是验证型 Ability，不足以代表正式闪避动作的长期承载需求。
3. 原因二：
   公共语义层的价值在于抽“多个动作已经证明稳定存在的共通规则”；
   如果现在就抽，容易把接口建立在“普攻视角 + 最小测试 Dodge 视角”上，抽得过早。
4. 原因三：
   正式闪避会带来方向、位移、冷却、无敌帧、状态限制、动作结束衔接等真实约束；
   这些约束没落地前，公共语义层很难一次抽准。
5. 原因四：
   最小预输入本质上依赖统一动作上下文、逻辑结束事件和输入评估；
   所以它也不适合排在公共语义层之前。
6. 因此当前更稳的顺序是：
   先做基础闪避正式落地；
   再做公共语义层；
   再做最小预输入。

# 为什么不是“先做预输入”

1. 预输入不是孤立功能，它依赖：
   当前动作是什么；
   当前动作处于哪个阶段；
   当前输入是该立即执行、进入缓存，还是直接拒绝；
   当前动作逻辑结束时由谁统一结算缓存。
2. 这些能力如果现在直接在普攻和闪避里各自实现，会很容易出现两套输入缓存逻辑。
3. 因此预输入更适合放在公共语义层之后，而不是放在基础闪避之前。

# 当前有效技术结构

1. 技术栈
   沿用 `UE5 + GAS + C++`，不跳出既定项目路线。
2. 角色侧负责：
   `ASC` 持有与初始化；
   默认 Ability 授予；
   输入转发；
   `NormalAttackMontage` 与 `NormalAttackSectionNames` 配置；
   调试快照承载与显示。
3. 普攻 Ability 侧负责：
   普攻段序进入；
   当前段 Montage 播放；
   段内再次输入接收；
   下一段推进；
   连段结束清理；
   普攻阶段维护；
   最小 Dodge 打断判定与执行；
   调试日志输出。
4. 长期方向
   普通攻击继续按“一段普攻一个 Ability”组织；
   后续闪避、格挡、受击和预输入，也应继续在 Ability 与公共语义层上扩展，不回退到 Character 级状态机。

# 当前关键接口现状

1. GAS 底座
   `AtwoheartsCharacter` 已实现 `IAbilitySystemInterface`；
   角色已持有并初始化 `AbilitySystemComponent`；
   默认战斗 Ability 授予链路已打通。
2. 普攻输入入口
   `AtwoheartsCharacter::NormalAttack`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
3. 普攻正式执行入口
   `UTwoHeartsGA_NormalAttackBase::ActivateAbility`
   `UTwoHeartsGA_NormalAttackBase::InputPressed`
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
4. 普攻阶段入口
   `UTwoHeartsGA_NormalAttackBase::EnterCombatPhase`
   `UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin`
   `UTwoHeartsGA_NormalAttackBase::NotifyCombatPhaseByName`
5. 最小 Dodge 打断入口
   `UTwoHeartsGA_NormalAttackBase::CanBeInterruptedByDodge`
   `UTwoHeartsGA_NormalAttackBase::TryInterruptByDodge`
   `UTwoHeartsGA_Dodge::ActivateAbility`
6. 当前调试状态入口
   `AtwoheartsCharacter::SetNormalAttackDebugRuntimeState`
   `AtwoheartsCharacter::DrawNormalAttackDebugOverlay`
   `ATwoheartsDebugHUD::DrawHUD`

# 当前使用中的关键 Tag

1. 已保留：
   `Ability.NormalAttack`
   `State.Action.NormalAttack`
   `State.CannotAttack`
   `State.CannotInput`
2. 已新增：
   `Ability.NormalAttack.Segment1`
   `Ability.NormalAttack.Segment2`
   `Ability.NormalAttack.Segment3`
3. 阶段命名方向已明确：
   `State.Phase.Startup`
   `State.Phase.Active`
   `State.Phase.Recovery`
   `State.Phase.LogicEnded`
4. 当前说明
   三段普攻 Ability 已通过独立 Ability Tag 识别与推进；
   阶段命名已明确，但是否全面写入公共 Tag，留待公共语义层统一收束。

# 普攻当前正式口径

1. 普攻首击只允许进入第 `1` 段。
2. 第 `1` 段和第 `2` 段执行期间再次点击，只缓存一次“下一段请求”。
3. 当前段结束后，如已请求下一段，则按 Tag 激活下一段 Ability。
4. 第 `3` 段结束后直接结束整套普攻。
5. Ability 激活后自动进入 `Startup`。
6. `Active / Recovery / LogicEnded` 优先由 Notify 驱动。
7. 若当前动画资源未补齐 Notify，则允许按 Section 时长做最小时序兜底。
8. `LogicEnded` 表示战斗规则层面的逻辑结束，不等同于 Montage 完全播完，也不等同于 Ability 生命周期结束。

# 普攻阶段通知规范

1. 当前标准 Notify 名称：
   `CombatPhase_Active`
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`
2. `Startup`
   不需要额外配置 Notify；
   当前由 Ability 激活成功后自动进入。
3. 配置原则
   `CombatPhase_Active` 放在动作真正开始生效的起点附近；
   `CombatPhase_Recovery` 放在有效段结束、收招开始的位置；
   `CombatPhase_LogicEnded` 放在逻辑上允许衔接下一动作的位置，不要简单贴到动画最后一帧。
4. 当前联调补充口径
   当前可先按单段逐个补 Notify，再进游戏验证 `phase / dodge / logic_end` 三个关键字段；
   若资源尚未补齐，代码仍会按 Section 时长兜底；
   后续正式联调应逐步转回 Notify 驱动。

# 下一直接开工项

1. 当前目标
   完成“基础闪避正式落地”，不是只保留当前最小 Dodge 打断验证。
2. 当前明确不做
   不在这一轮就把公共语义层一起抽完；
   不在这一轮就接通完整预输入；
   不在这一轮同时把受击、伤害、格挡一起揉进来。
3. 这一轮应补齐的基础闪避能力
   正式 Dodge 输入入口；
   基础方向判定；
   基础位移/动画承载；
   基础冷却与限制 Tag；
   基础无敌帧状态表达；
   闪避开始、执行中、结束后的最小调试与联调输出；
   与当前普攻阶段语义的正式打断衔接。
4. 这一轮优先复用的现有接口
   `HandleAbilityInputPressed`
   当前普攻阶段查询与 Dodge 打断入口；
   已有的 GAS Ability 授予和 Tag 出口；
   当前调试面板与屏幕覆盖调试机制。
5. 这一轮资深程序应避免的做法
   不要把闪避状态机写回 `AtwoheartsCharacter`；
   不要为了闪避先提前造完整公共语义层；
   不要在闪避里单独发明一套与普攻无关的阶段、打断和输入缓存规则。

# 基础闪避完成后的开工条件

1. 当基础闪避已经形成正式 Ability 承载后，主程序再 review：
   普攻和闪避是否已经共享一批稳定存在的概念；
   当前是否已经足够支撑统一动作上下文、统一打断规则和统一逻辑结束事件。
2. 如果结论成立，再进入：
   [a基础战斗公共语义层技术文档](./a基础战斗公共语义层技术文档.md)

# 已完成内容留档

1. 2026-05-05：最小普通攻击闭环第一版完成。
   结果：
   角色已具备最基础的 `1-2-3` 普通攻击连段能力；
   当时实现位于 `AtwoheartsCharacter`，用于验证行为闭环，不是最终承载结构。
2. 2026-05-14：`AbilitySystem` 基础底座接入完成。
   结果：
   工程已接通 `GameplayAbilities`、`GameplayTags`、`GameplayTasks`；
   角色已持有并初始化 `AbilitySystemComponent`；
   默认战斗 Ability 授予链路、基础 Ability 基类和最小 Gameplay Tag 出口已建立。
3. 2026-05-15：普通攻击正式迁入 `AbilitySystem` 完成。
   结果：
   普攻输入、段序驱动、Montage 播放、段间输入缓存、下一段激活和主要调试状态已迁入 Ability；
   旧测试 Ability、旧 Character 普攻状态机和新旧切换开关已退出正式路径。
4. 2026-05-17：普通攻击阶段标记与基础打断完成。
   结果：
   普攻三段 Ability 已具备 `Startup / Active / Recovery / LogicEnded` 阶段运行态；
   阶段切换支持 Montage Notify 驱动，并保留最小时序兜底；
   最小 Dodge Ability 与阶段打断判定已接通；
   调试面板、结构化日志和屏幕覆盖调试信息已补齐阶段与打断状态输出。
5. 2026-05-17：阶段资源联调与调试口径收束完成。
   结果：
   普攻阶段资源已开始按 Notify 规范配置；
   `Dodge` 输入映射已可接入最小打断链路；
   调试输出已收敛为以 `phase / dodge / logic_end` 和关键事件为主；
   当前可验证“打断逻辑是否成立”，但不应把“无正式闪避表现资源”误判为打断失败。
6. 2026-05-17：基础闪避正式落地 `C++` 第一版完成。
   结果：
   `UTwoHeartsGA_Dodge` 已从“最小打断验证 Ability”扩展为正式基础闪避承载；
   当前已接通闪避方向解析、基础 `8` 方向命名、最小位移推进、冷却 Tag、无敌帧 Tag、动作开始到结束的生命周期日志；
   普攻 `Recovery / LogicEnded` 可被闪避打断，`Startup / Active` 默认仍不可被闪避直接打断；
   `AtwoheartsCharacter` 已补角色侧闪避配置入口和调试承载，`HUD` 已可显示 `dodging / dodge_direction / dodge_invulnerable / dodge_cooldown_ready`；
   当前 `UnrealBuildTool` 编译通过，但仍待蓝图资源配置与场景联调后再做阶段验收结论。
7. 2026-05-17：基础闪避资源承载升级为 `8` 向 Montage 配置。
   结果：
   角色侧 `DodgeConfig` 已支持 `Forward / ForwardRight / Right / BackwardRight / Backward / BackwardLeft / Left / ForwardLeft` 八方向独立 Montage 槽位；
   同时保留 `DodgeMontageFallback` 作为未补齐方向资源时的兜底；
   `UTwoHeartsGA_Dodge` 会按当前解析出的方向名自动选择对应 Montage 播放。
8. 2026-05-17：基础闪避第一轮 Bug 修复与 Root Motion / Notify 正式化完成。
   结果：
   闪避冷却清理改为依赖缓存 ASC，不再依赖 Ability 结束后的 `ActorInfo` 临时取值；
   移动输入已补 `Completed / Canceled` 清零，站定闪避不会再错误沿用旧方向；
   闪避开始前会缓存角色原本旋转/移动模式，结束时统一恢复，不再写死回默认模式；
   收尾逻辑已统一收束到 `EndAbility` 清理路径，正常结束和取消结束共用同一套恢复流程；
   正式闪避位移承载已统一收束为 `Root Motion` 主路径，若方向 Montage 未启用 Root Motion，程序会直接拒绝本次正式闪避；
   无敌帧开始、无敌帧结束和动作逻辑结束已升级为 Notify 主驱动，并保留时间窗作为未补齐 Notify 时的最小兜底；
   `UnrealBuildTool` 编译通过，当前可进入下一轮游戏内跑测。

# 当前实际代码落点

1. 角色入口与调试承载：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`
2. 普攻阶段枚举：
   `twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`
3. 普攻 Ability 基类：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
4. 三段正式普攻 Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.cpp`
5. 正式基础 Dodge Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
6. 基础 Ability 与 Tag 出口：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`

# 已删除或退出正式路径的旧内容

1. 已删除测试 Ability：
   `UTwoHeartsGA_TestNormalAttack`
2. 已退出正式路径的旧 Character 普攻函数：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
3. 已删除旧 Character 普攻运行态：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
   `NormalAttackSegmentTimerHandle`
4. 已删除新旧切换开关：
   `bUseAbilitySystemForNormalAttackInput`

# 踩坑记录与注意事项

1. 当前“第 2 段看起来没播出”的历史问题，最终定位是动画资源表现不够易区分，不是 GAS 段切换失败。
2. 如果日志里已经出现：
   `PlaySegment segment=2 section=Attack_2`
   应优先怀疑资源表现和 Section 落点，而不是先怀疑 Ability 段切换逻辑。
3. 当前代码虽已支持 Notify 驱动，但仍保留最小时序兜底；
   后续联调应逐步补齐动画通知，避免长期依赖纯时长推断。
4. 当前正式闪避已经按 `Root Motion + Notify` 口径收束；若某个方向资源未启用 Root Motion，程序会直接拒绝该次闪避并输出黄色日志。
5. 当前若看到 `Dodge` 输入已进入日志、普攻可在允许阶段被截断，但闪避未正式触发，优先排查当前方向 Montage 是否已配置 Root Motion，以及 `Dodge_InvulnerableBegin / Dodge_InvulnerableEnd / Dodge_Finished` 是否已正确配置。
6. 后续如继续扩战斗规则，应优先复用当前 Ability 承载与阶段语义出口，不要新开 Character 级临时状态机。

# 当前未完成项与技术债

1. 基础闪避第二轮本地联调验收尚未完成。
2. 正式通用预输入尚未完成。
3. 公共战斗语义层尚未完成。
4. 受击、伤害、格挡联动尚未完成。
5. 联机同步尚未处理。
6. Ability 结束与 Montage 收尾的一致性，后续仍需在公共语义层阶段继续统一。
7. `ActorInfo` 初始化与默认 Ability 授予链路，对未来双人共斗和联机扩展的鲁棒性，后续仍需复查。
