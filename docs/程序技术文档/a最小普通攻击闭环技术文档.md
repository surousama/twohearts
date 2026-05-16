# 文档用途

1. 这份文档用于留存《双心印》第二章第一阶段“最小普通攻击闭环”的实现结果。
2. 当前文档不再作为直接开工文档，主要用于回看实现、联调与后续迁移。

# 基础信息

1. 功能名称：最小普通攻击闭环
2. 对应设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2.4 节
3. 对应技术总文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4. 实现日期：2026-05-05
5. 当前状态：已完成第一版本地最小闭环

# 本阶段完成结果

1. 角色已具备最基础的 1-2-3 普通攻击连段能力。
2. 普通攻击输入、段序推进、连段结束重置已在 C++ 中跑通。
3. 当前方案可用于第二章后续战斗功能验证，但属于过渡实现，不是最终架构。

# 本次实际实现

1. 新增普通攻击输入入口 `NormalAttackAction`。
2. 新增普通攻击配置入口 `NormalAttackMontage` 与 `NormalAttackSectionNames`。
3. 在角色类中集中维护最小普攻状态：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
4. 实现以下核心函数：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
5. 补充普通攻击调试能力：
   结构化日志
   游戏内调试面板
   控制台调试开关

# 核心代码位置

1. `twohearts/Source/twohearts/twoheartsCharacter.h`
2. `twohearts/Source/twohearts/twoheartsCharacter.cpp`
3. `twohearts/Source/twohearts/twoheartsDebugHUD.h`
4. `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`
5. `twohearts/Source/twohearts/twoheartsPlayerController.h`
6. `twohearts/Source/twohearts/twoheartsPlayerController.cpp`

# 当前资源与配置

1. 当前联调角色：`ChineseWarrior`
2. 当前普攻 Montage：
   `Chinese_Warrior/Animations/Combat_Montage/AM_Melee_NormalAttackCombo`
3. 当前 Section：
   `Attack_1`
   `Attack_2`
   `Attack_3`
4. 当前 Sequence：
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_01`
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_02`
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_03`
5. 当前输入资产：
   `IA_NormalAttack`

# 验证口径

1. 点一次普攻只播第 1 段。
2. 连续点击能稳定播出 1-2-3。
3. 第 2 段或第 3 段结束后不继续点能正确回待机。
4. 连段结束后再次点击能重新从第 1 段开始。

# 当前已知限制

1. 当前实现仍放在 `AtwoheartsCharacter`，尚未接入正式 AbilitySystem 架构。
2. 当前段落推进依赖 Section 时长和 Timer，不是最终方案。
3. 当前还没有正式阶段语义、打断、受击、伤害、预输入和通用 Tag 体系。

# 后续迁移注意事项

1. 后续应先接入 AbilitySystem 基础底座，再把本闭环迁入 Ability 或角色战斗组件。
2. 迁移时应保留当前 1-2-3 行为结果，不要在同一阶段同时叠加太多新玩法规则。
3. 进入正式阶段语义后，应把当前基于 Timer 的推进改为动画通知或动作逻辑事件驱动。

# 相关文档

1. 下一阶段历史留档文档：[aAbilitySystem基础底座实现文档](./aAbilitySystem基础底座实现文档.md)
2. 后续收束文档：[a基础战斗公共语义层技术文档](./a基础战斗公共语义层技术文档.md)
