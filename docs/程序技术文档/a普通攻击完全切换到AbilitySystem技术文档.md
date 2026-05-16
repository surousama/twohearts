# 文档用途

1. 这份文档用于给资深程序直接开工，完成《双心印》当前阶段“普通攻击完全切换到 AbilitySystem”的实现。
2. 本文档是第二章当前新的直接实施文档，目标不是继续验证 GAS 是否可用，而是结束普通攻击“新旧两套逻辑并存”的过渡状态。
3. 本文档默认以上一阶段 `aAbilitySystem基础底座实现文档.md` 已落地为前提。

# 基础信息

1. 功能名称：普通攻击完全切换到 AbilitySystem
2. 上游设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2.4 节
3. 上游总览文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4. 上一阶段合并留档文档：[aAbilitySystem基础底座实现文档](./aAbilitySystem基础底座实现文档.md)
5. 当前验收口径：单角色本地验证
6. 当前阶段目标：游戏内普通攻击正式只保留 AbilitySystem 一套逻辑

# 当前主程序结论

1. 当前没有阻断本阶段开工的严重漏洞。
2. 当前“输入走 GAS、连段逻辑仍在 Character”的实现，可以视为上一阶段可接受的过渡实现。
3. 但当前实现已经不适合继续叠加普通攻击后续功能。
4. 原因不是它现在完全不能用，而是它的正式承载位置不对：
   普攻输入入口已经进入 `AbilitySystemComponent`；
   但段序推进、Montage 播放、Timer 驱动、重置和调试状态仍主要压在 `AtwoheartsCharacter`。
5. 如果继续在这套 Character 级临时状态机上叠加阶段语义、打断、预输入、受击联动，后续一定发生二次迁移。
6. 因此本阶段正确顺序不是继续补功能，而是先把普通攻击完整迁入 AbilitySystem，再往上叠加后续规则。

# 本阶段完成标准

1. 游戏内普通攻击只有一套正式逻辑。
2. 普通攻击输入不再切回旧的 `TryStartNormalAttack` 路径。
3. 当前 1-2-3 普攻最小闭环，能够在 GAS 上稳定复现。
4. 第 1 段、第 2 段、第 3 段的进入、衔接、结束重置，都由 Gameplay Ability 主导。
5. `AtwoheartsCharacter` 不再承担普通攻击的段序状态机职责。
6. 旧测试 Ability 和旧 Character 普攻逻辑，不再作为游戏内正式路径保留。

# 本阶段明确不做

1. 不在本阶段引入正式预输入系统。
2. 不在本阶段引入完整的 `Startup / Active / Recovery / LogicEnded` 公共阶段语义。
3. 不在本阶段引入正式受击、伤害、闪避、格挡联动。
4. 不在本阶段重构新的战斗角色基类。
5. 不在本阶段处理完整联机同步。
6. 不在本阶段顺手扩写成通用战斗组件大重构。

# 当前代码现状

1. `AtwoheartsCharacter::NormalAttack` 已能把输入送进 `HandleAbilityInputPressed(ETwoHeartsAbilityInputID::NormalAttack)`。
2. 当前默认授予的普攻 Ability 还是 `UTwoHeartsGA_TestNormalAttack`，激活后只打印日志并立即结束。
3. 旧普攻 1-2-3 的实际逻辑仍在 `AtwoheartsCharacter`：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
4. 当前旧逻辑仍依赖：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
   `NormalAttackSegmentTimerHandle`
5. 当前动画配置仍在角色侧：
   `NormalAttackMontage`
   `NormalAttackSectionNames`
6. 当前调试面板与日志也仍主要围绕旧 Character 状态机输出。

# 当前阶段的正式承载结论

1. 本阶段普通攻击的正式逻辑承载位置，应切到 Gameplay Ability。
2. 当前阶段不要求马上抽出完整战斗组件，但禁止继续把新的普攻状态机写回 `AtwoheartsCharacter`。
3. 角色类本阶段只保留以下职责：
   持有 `AbilitySystemComponent`；
   处理普通攻击输入绑定；
   授予默认普通攻击 Ability；
   持有角色级动画资源配置。
4. 普通攻击 Ability 本阶段负责以下职责：
   输入触发后的段序进入；
   当前段 Montage 播放；
   连段输入缓存；
   当前段结束后的下一段衔接；
   连段结束清理；
   普攻调试日志的正式输出。
5. 当前阶段允许“角色持有配置，Ability 持有逻辑”的最小分工。
6. 当前阶段不建议为了这一次普攻替换，额外硬造新的战斗组件层；这类统一收束放到后续公共语义层阶段处理更稳。

# 建议实现结构

1. 正式普攻基类：
   `UTwoHeartsGA_NormalAttackBase`
2. 三段最小普攻 Ability：
   `UTwoHeartsGA_NormalAttack_1`
   `UTwoHeartsGA_NormalAttack_2`
   `UTwoHeartsGA_NormalAttack_3`
3. 当前测试 Ability：
   `UTwoHeartsGA_TestNormalAttack`
   处理建议：
   不再作为正式默认普攻入口；
   如果确认无后续测试用途，直接删除更干净。
4. 推荐目录：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/`
5. 当前角色入口保留位置：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`

# 建议职责拆分

1. `UTwoHeartsGA_NormalAttackBase`
   负责共享普通攻击通用流程：
   读取角色与 ASC；
   读取当前段配置；
   播放 Montage Section；
   记录本段是否收到下一次普攻输入；
   在本段结束时决定进入下一段还是结束；
   输出统一调试日志。
2. `UTwoHeartsGA_NormalAttack_1`
   负责第 1 段的正式入口。
3. `UTwoHeartsGA_NormalAttack_2`
   负责第 2 段。
4. `UTwoHeartsGA_NormalAttack_3`
   负责第 3 段和连段收尾。
5. `AtwoheartsCharacter`
   不再持有旧普攻运行状态；
   只负责输入转发、角色资源配置和默认 Ability 授予。

# 段序驱动建议

1. 当前阶段继续沿用“一段普攻一个 Ability”的长期方向，不回退成单一大 Ability 模式。
2. 普攻按下时：
   如果当前没有普通攻击段在执行，则激活第 1 段 Ability。
3. 当第 1 段或第 2 段执行期间再次收到普通攻击输入时：
   当前激活中的普通攻击 Ability 只记录“已请求下一段”。
4. 当前段结束时：
   如果已请求下一段且存在下一段 Ability，则激活下一段；
   否则结束普通攻击并回待机。
5. 第 3 段结束时：
   直接结束整套普通攻击。
6. 本阶段不做“多输入排队”，只保留最小的“是否请求下一段”即可。

# 动画与时序建议

1. 当前阶段正式逻辑迁入 GAS 后，Montage 播放应由 Ability 发起，而不是由 Character 主动发起。
2. 如果当前项目已能稳定使用 Ability 侧的 Montage 播放与结束回调，优先使用 GAS/Ability 自身的播放与等待链路。
3. 如果当前阶段暂时还需要最小时间驱动，也应把这部分时序控制留在 Ability 内，不再回写到 Character 普攻函数。
4. 当前阶段仍允许继续使用现有 Montage 和 `Attack_1 / Attack_2 / Attack_3` Section 资源。
5. 当前角色上的 `NormalAttackMontage` 和 `NormalAttackSectionNames`，本阶段可以继续作为配置入口保留，不强制同时迁成独立数据资产。
6. 但无论配置放在哪里，普攻逻辑控制权必须在 Ability。

# Tag 与激活建议

1. 保留当前已有的：
   `Ability.NormalAttack`
   `State.Action.NormalAttack`
   `State.CannotAttack`
   `State.CannotInput`
2. 本阶段建议补充每段独立识别标签：
   `Ability.NormalAttack.Segment1`
   `Ability.NormalAttack.Segment2`
   `Ability.NormalAttack.Segment3`
3. 每段 Ability 需要有独立可识别的 Asset Tag，避免后续段序激活时继续靠类名硬判断。
4. 本阶段不要求一次补齐完整公共动作阶段 Tag。

# 旧逻辑清理要求

1. 从正式默认授予列表中移除 `UTwoHeartsGA_TestNormalAttack`。
2. 删除或停用 `bUseAbilitySystemForNormalAttackInput` 这种“新旧切换开关”。
3. 删除旧普攻正式入口：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
4. 删除旧普攻运行态成员：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
   `NormalAttackSegmentTimerHandle`
5. 如果当前调试 HUD 仍强依赖旧字段，需同步迁到新普攻 Ability 的调试来源；不要为了保面板而继续留旧状态机。
6. 最终验收时，游戏内不得再存在“把普攻切回旧 Character 逻辑”的正式入口。

# 推荐施工顺序

1. 先新增正式普通攻击基类和三段 Ability 类。
2. 把默认普攻授予从 `UTwoHeartsGA_TestNormalAttack` 切到正式第 1 段入口。
3. 在 Ability 内复现当前 1-2-3 连段最小闭环。
4. 让 Character 的普通攻击输入只保留 GAS 输入转发。
5. 迁移调试日志与必要调试显示。
6. 最后删除旧 Character 普攻状态机与切换开关。

# 施工注意事项

1. 这次目标是“替换正式承载位置”，不是“顺手设计完整战斗体系”。
2. 不要一边迁移普攻，一边把闪避、受击、打断、预输入一起揉进去。
3. 不要为了追求一步到位，临时新增比当前问题更大的抽象层。
4. 角色侧遗留的普攻配置可以暂留，角色侧遗留的普攻状态机不能暂留。
5. 如果出现“新 Ability 能播第 1 段，但段间输入进不到当前激活 Ability”的问题，优先修 GAS 输入转发和 Ability 内输入接收，不要退回 Character 补一层手动状态机。
6. 如果调试面板迁移成本较高，本阶段可先保留日志为主、面板简化为辅，但不能因此阻止旧逻辑删除。

# 联调与验收口径

1. 点击一次普通攻击，只播放第 1 段。
2. 连续点击普通攻击，能够稳定播出 1-2-3。
3. 第 1 段结束前未继续点击时，角色正确回待机。
4. 第 2 段结束前未继续点击时，角色正确回待机。
5. 第 3 段结束后，角色正确结束整套普通攻击并回待机。
6. 连段结束后再次点击，能够重新从第 1 段开始。
7. 普攻输入全程由 AbilitySystem 驱动，不再回落到旧 Character 路径。
8. 代码中不再保留正式可用的新旧双路径切换。

# 风险点

1. 最大风险不是第 1 段播不出来，而是“输入能激活第 1 段，但当前段执行中新的输入无法正确传给激活中的 Ability”。
2. 第二个风险是“为了保旧 HUD 或保旧蓝图配置，最后旧状态机删不干净”，导致项目表面上已迁移，实际上仍双轨并存。
3. 第三个风险是“把每段 Ability 做出来了，但下一段激活仍依赖 Character 帮忙判断”，这不算真正完成替换。
4. 第四个风险是“为了快，直接把三段重新揉回一个测试 Ability”，这会和上游框架文档冲突，也会把后续扩展点做窄。

# 完成后需要回写的文档

1. 在本文件内补充“实际代码文件、关键函数、最终实现口径、已知限制”。
2. 更新 [a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md) 的当前入口和下一步顺序。
3. 更新 [a双心印开发进度简介](../a双心印开发进度简介.md) 中与普通攻击阶段相关的状态描述。

# 当前已落地实现记录

1. 实现日期：
   2026-05-15
2. 本次实际完成结论：
   普通攻击正式承载已从 `AtwoheartsCharacter` 迁入 Gameplay Ability；
   游戏内普通攻击正式只保留 GAS 一套路径；
   旧测试 Ability 和旧 Character 普攻状态机已退出正式路径。
3. 当前阶段验收口径仍为：
   单角色本地验证。

# 本次实际代码落点

1. 角色入口与调试承载：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
2. 调试面板：
   `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`
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
5. Gameplay Tag 扩展：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`
6. 基础 Ability 公共能力补充：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.cpp`

# 本次删除与停用内容

1. 已删除测试 Ability：
   `UTwoHeartsGA_TestNormalAttack`
2. 已删除旧 Character 普攻正式入口：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
3. 已删除旧 Character 普攻运行态成员：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
   `NormalAttackSegmentTimerHandle`
4. 已删除新旧切换开关：
   `bUseAbilitySystemForNormalAttackInput`

# 当前最终实现口径

1. `AtwoheartsCharacter`
   当前只保留：
   `ASC` 持有与初始化；
   默认 Ability 授予；
   普攻输入转发到 `HandleAbilityInputPressed`；
   `NormalAttackMontage` 与 `NormalAttackSectionNames` 配置入口；
   普攻调试运行态与调试事件承载。
2. `UTwoHeartsGA_NormalAttackBase`
   当前负责：
   统一普通攻击激活流程；
   读取角色 Montage 与 Section；
   发起 Montage 播放；
   接收当前段期间的再次普攻输入；
   记录“是否请求下一段”；
   在当前段结束后决定是否激活下一段；
   输出统一调试日志与失败信息。
3. `UTwoHeartsGA_NormalAttack_1 / 2 / 3`
   当前负责：
   声明各自段号；
   声明各自独立识别 Tag；
   声明下一段激活目标。
4. 段序驱动口径
   第一次点击普攻时，由第 1 段 Ability 激活；
   第 1 段或第 2 段执行期间再次点击普攻，只记录“请求下一段”；
   当前段 Montage 完成后，如已请求下一段，则按 Tag 激活下一段 Ability；
   第 3 段结束后直接结束整套普通攻击。
5. 当前时序口径
   不再使用 Character 侧 Timer；
   当前改为 Ability 内部通过 `UAbilityTask_PlayMontageAndWait` 的完成/打断回调驱动段落收束。

# 关键函数与逻辑链条

1. 输入入口：
   `AtwoheartsCharacter::NormalAttack`
   作用：
   普攻输入只进入 GAS 输入桥接，不再回落旧 Character 普攻逻辑。
2. GAS 输入分发：
   `AtwoheartsCharacter::HandleAbilityInputPressed`
   作用：
   优先把输入发送给当前已激活的同 InputID Ability；
   若当前没有激活中的同输入 Ability，再尝试激活可用 Ability。
3. 普攻段开始：
   `UTwoHeartsGA_NormalAttackBase::ActivateAbility`
   作用：
   提交 Ability；
   初始化本段运行态；
   进入 Montage 播放。
4. 当前段播放：
   `UTwoHeartsGA_NormalAttackBase::StartSegmentPlayback`
   作用：
   校验角色、Montage、AnimInstance、Section；
   发起当前段 Montage 播放；
   写入当前调试状态。
5. 当前段期间再次点击：
   `UTwoHeartsGA_NormalAttackBase::InputPressed`
   作用：
   第 1 段和第 2 段允许缓存一次“下一段请求”；
   第 3 段不再接受下一段请求。
6. 当前段结束：
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
   作用：
   处理正常结束或被打断；
   若已请求下一段，则按 `NextSegmentAbilityTag` 激活下一段；
   否则结束当前 Ability 并清空调试运行态。
7. 调试状态同步：
   `AtwoheartsCharacter::SetNormalAttackDebugRuntimeState`
   作用：
   用于 HUD 和结构化日志读取当前普攻运行态快照。

# 当前 Tag 使用口径

1. 保留原有：
   `Ability.NormalAttack`
   `State.Action.NormalAttack`
   `State.CannotAttack`
   `State.CannotInput`
2. 本次新增：
   `Ability.NormalAttack.Segment1`
   `Ability.NormalAttack.Segment2`
   `Ability.NormalAttack.Segment3`
3. 当前用途：
   用于标识三段普攻 Ability；
   用于 Ability 间按 Tag 激活下一段；
   为后续阶段语义与打断规则继续扩展预留稳定出口。

# 当前调试与测试入口说明

1. 现有调试面板仍可用。
2. 但当前调试来源已不再是旧 Character 状态机，而是 Ability 运行态快照与事件记录。
3. 当前调试面板主要关注字段：
   `attacking`
   `segment`
   `queued_next`
   `latest_section`
   `Last Failure`
   `Recent Events`
4. 当前常见事件包括：
   `PlaySegment`
   `QueueNextSegment`
   `SegmentFinished`
   `AdvanceSegment`
   `SegmentInterrupted`
   `ActivateFailed`
5. 当前如果看到 Ability 激活失败，优先检查：
   `NormalAttackMontage` 是否已配置；
   `Attack_1 / Attack_2 / Attack_3` Section 是否存在；
   角色 AnimInstance 是否正常。

# 交接给资深测试的最小验证清单

1. 点击一次普通攻击，只播放第 1 段。
2. 在第 1 段期间再次点击，能够稳定进入第 2 段。
3. 在第 2 段期间再次点击，能够稳定进入第 3 段。
4. 第 1 段结束前未继续点击时，角色正确回待机。
5. 第 2 段结束前未继续点击时，角色正确回待机。
6. 第 3 段结束后，角色正确结束整套普通攻击并回待机。
7. 连段完整结束后再次点击，能够重新从第 1 段开始。
8. 快速连续点击时，当前阶段只应缓存一个“下一段请求”，不应出现多段排队。
9. 普攻输入过程中，不应再回落到旧 Character 普攻路径。
10. 调试面板中的 `segment`、`queued_next`、`latest_section` 应与实际播放段落一致。

# 当前已完成的程序侧验证

1. 已使用以下命令完成工程编译验证：
   `H:\UE_5.6\Engine\Build\BatchFiles\Build.bat twoheartsEditor Win64 Development H:\twohearts\twohearts\twohearts.uproject -WaitMutex -NoHotReloadFromIDE`
2. 当前编译结果：
   成功。
3. 当前尚未完成的验证：
   尚未在编辑器内逐条执行完整手动战斗联调；
   因此下一步需要资深测试补齐游戏内行为验证。

# 合并测试说明：GAS 底座 + 普攻切换

1. 本节用途：
   把 `aAbilitySystem基础底座实现文档.md` 与本文件的测试重点合并为一套联调顺序；
   方便策划或资深测试一次性验证“底座是否真的接通”以及“普通攻击是否真的已切到 GAS”。
2. 当前正确理解口径：
   现在不是只测“普通攻击能不能播动画”；
   而是要一起确认：
   角色是否成功接入 GAS；
   输入是否真的走 Ability 激活链路；
   普攻 1-2-3 是否已经由 Gameplay Ability 正式承载。

# 先测什么，后测什么

1. 第一步先测 GAS 底座是否活着。
2. 第二步再测普通攻击是否建立在这个底座之上。
3. 如果第一步没过，第二步的很多现象都会失真，不建议直接跳过底座验证。

# 一体化测试前提

1. 当前工程应能成功编译并启动编辑器。
2. 当前角色应为 `AtwoheartsCharacter` 体系。
3. 当前角色蓝图或角色实例应已正确配置：
   `NormalAttackAction`
   `NormalAttackMontage`
   `NormalAttackSectionNames`
4. 当前 `NormalAttackSectionNames` 默认预期为：
   `Attack_1`
   `Attack_2`
   `Attack_3`
5. 当前 Montage 内应真实存在以上 3 个 Section。
6. 当前项目应已启用 `GameplayAbilities` 插件，并已补齐：
   `GameplayAbilities`
   `GameplayTags`
   `GameplayTasks`

# 阶段A：GAS 底座验证

1. 进入游戏后，角色应能正常生成，不应因 AbilitySystem 初始化报错而导致角色不可用。
2. 普攻输入按下后，不应出现“完全没有进入 AbilitySystem”的表现。
3. 如果打开日志，当前应至少能确认：
   `AbilitySystem` 已初始化 ActorInfo；
   默认战斗 Ability 已授予；
   普攻输入已进入 `HandleAbilityInputPressed` 链路。
4. 当前角色应具备稳定可访问的 `AbilitySystemComponent`。
5. 当前默认普攻 Ability 不再是旧测试 Ability，而应是正式普攻段 Ability 列表。

# 阶段A失败时优先检查

1. `twohearts.Build.cs` 是否已补齐 GAS 模块依赖。
2. `twohearts.uproject` 是否已启用 `GameplayAbilities` 插件。
3. `AtwoheartsCharacter` 是否已实现 `IAbilitySystemInterface`。
4. `AbilitySystemComponent` 是否已创建并可返回。
5. `BeginPlay` 中是否先初始化 `ActorInfo`，再授予默认 Ability。
6. 默认 Ability 列表里是否已是：
   `UTwoHeartsGA_NormalAttack_1`
   `UTwoHeartsGA_NormalAttack_2`
   `UTwoHeartsGA_NormalAttack_3`

# 阶段B：普通攻击正式切换验证

1. 点击一次普通攻击，只播放第 1 段。
2. 在第 1 段期间再次点击，能够稳定进入第 2 段。
3. 在第 2 段期间再次点击，能够稳定进入第 3 段。
4. 第 1 段结束前未继续点击时，角色正确回待机。
5. 第 2 段结束前未继续点击时，角色正确回待机。
6. 第 3 段结束后，角色正确结束整套普通攻击并回待机。
7. 连段完整结束后再次点击，能够重新从第 1 段开始。
8. 快速连续点击时，当前阶段只应缓存一个“下一段请求”，不应出现多段排队。
9. 普攻输入过程中，不应再回落到旧 Character 普攻路径。
10. 代码和运行结果都不应再依赖：
    `TryStartNormalAttack`
    `PlayNormalAttackSegment`
    `HandleNormalAttackSegmentFinished`
    `ResetNormalAttackCombo`

# 阶段B失败时优先检查

1. `NormalAttackMontage` 是否为空。
2. `Attack_1 / Attack_2 / Attack_3` Section 是否缺失或命名不一致。
3. 角色 `AnimInstance` 是否正常存在。
4. 当前激活中的 Ability 是否真的收到了再次按下的输入。
5. 下一段 Ability 是否能按：
   `Ability.NormalAttack.Segment1`
   `Ability.NormalAttack.Segment2`
   `Ability.NormalAttack.Segment3`
   正确识别和激活。

# 调试面板阅读说明

1. 当前调试面板已从“旧 Character 状态机面板”切换为“Ability 运行态面板”。
2. `attacking=true`
   表示当前有普攻 Ability 正在执行。
3. `segment`
   表示当前执行到第几段。
4. `queued_next=true`
   表示当前段执行期间已经收到一次下一段请求。
5. `latest_section`
   应与当前实际播放的 Montage Section 一致。
6. `Last Failure`
   用于快速判断是 Montage、Section、AnimInstance 还是下一段激活失败。
7. `Recent Events`
   用于回看本次输入和段落衔接顺序。

# 当前测试时最容易误判的点

1. 如果角色能移动，但按普攻完全没反应，不一定是普攻逻辑本身问题，也可能是 GAS 底座初始化链路没打通。
2. 如果第 1 段能播，但第 2 段接不上，优先怀疑“激活中的 Ability 没收到再次输入”，而不是先怀疑动画资源。
3. 如果日志里出现 `ActivateFailed`，优先检查角色资源配置，而不是先怀疑 ASC。
4. 如果面板显示 `queued_next=true` 但没有进入下一段，优先检查下一段 Ability Tag 激活链路。
5. 不要再用“旧 Character 普攻是否还可切回”作为当前正确行为判断标准，因为正式路径已经切干净了。

# 本次用于联调的核心代码入口

1. 底座初始化入口：
   `AtwoheartsCharacter::BeginPlay`
   `AtwoheartsCharacter::InitializeAbilitySystem`
   `AtwoheartsCharacter::GrantDefaultCombatAbilities`
2. 普攻输入入口：
   `AtwoheartsCharacter::NormalAttack`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
3. 普攻正式执行入口：
   `UTwoHeartsGA_NormalAttackBase::ActivateAbility`
   `UTwoHeartsGA_NormalAttackBase::InputPressed`
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
4. 调试状态入口：
   `AtwoheartsCharacter::SetNormalAttackDebugRuntimeState`
   `ATwoheartsDebugHUD::DrawHUD`

# 当前三份文档的合并使用结论

1. `aAbilitySystem基础底座实现文档.md`
   现在同时承担“为什么要先接 GAS”和“底座实际怎么接入”的历史留档作用。
2. 本文件
   现在已经补齐为当前最适合直接测试的主文档；
   如果你只想带着一份文档进编辑器联调，优先看本文件即可。

# 当前已知限制与后续注意事项

1. 本阶段只完成了“普通攻击正式迁入 AbilitySystem”，没有实现正式预输入。
2. 当前阶段没有实现完整的 `Startup / Active / Recovery / LogicEnded` 阶段语义。
3. 当前阶段没有实现正式打断关系、受击、伤害、闪避、格挡联动。
4. 当前阶段没有处理完整联机同步。
5. 当前段落切换虽然已迁入 Ability，但仍依赖 Montage 播放完成回调，不代表公共动作语义层已经完成。
6. 下一阶段不应再回到 Character 补普攻状态机，而应继续在 Ability 方向上补“阶段标记与基础打断”。
