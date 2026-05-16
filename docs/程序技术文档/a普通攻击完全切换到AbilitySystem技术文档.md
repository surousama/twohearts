# 文档用途

1. 这份文档用于给资深程序直接开工，完成《双心印》当前阶段“普通攻击完全切换到 AbilitySystem”的实现。
2. 本文档是第二章当前新的直接实施文档，目标不是继续验证 GAS 是否可用，而是结束普通攻击“新旧两套逻辑并存”的过渡状态。
3. 本文档默认以上一阶段 `aAbilitySystem基础底座实现文档.md` 已落地为前提。

# 基础信息

1. 功能名称：普通攻击完全切换到 AbilitySystem
2. 上游设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2.4 节
3. 上游总览文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4. 上一阶段实现文档：[aAbilitySystem基础底座实现文档](./aAbilitySystem基础底座实现文档.md)
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
