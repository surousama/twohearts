# 文档用途

1. 这份文档用于留存《双心印》第二章“AbilitySystem 基础底座”阶段的技术结论与实际实现结果。
2. 当前文档是底座阶段的合并留档文档，不再作为直接开工文档。
3. 主要用于回看当时为什么先接 GAS、底座怎么落地、初始化链路怎么组织，以及它和后续普通攻击迁移的关系。

# 基础信息

1. 功能名称：AbilitySystem 基础底座
2. 对应技术总文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
3. 对应设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2 章
4. 实现日期：2026-05-14
5. 当前状态：已完成并进入历史合并留档

# 本阶段原始目标与技术结论

1. 本阶段原始目标
   让角色正式具备 `AbilitySystemComponent` 接入能力；
   建立统一战斗 Ability 基类；
   建立最小 Native Gameplay Tags 出口；
   建立从输入语义到 Ability 激活的统一桥接结构；
   为下一阶段“普通攻击正式迁入 AbilitySystem”提供稳定承载基础。
2. 本阶段长期技术路线
   第二章正式技术路线确认沿用 `UE5 + GAS + C++`。
3. 本阶段核心结论
   `AbilitySystemComponent` 先挂在 `AtwoheartsCharacter` 上；
   角色初始化链路应先完成 `ActorInfo` 初始化，再授予默认战斗 Ability；
   输入桥接直接按长期方案建立，不走临时测试短线；
   战斗 Tag 从这一阶段开始统一注册与统一出口；
   普通攻击长期结构按“一段普攻一个 Ability”预留。

# 本阶段实际完成内容

1. `twohearts.Build.cs` 已补齐：
   `GameplayAbilities`
   `GameplayTags`
   `GameplayTasks`
2. `twohearts.uproject` 已启用：
   `GameplayAbilities` 插件。
3. `AtwoheartsCharacter` 已实现 `IAbilitySystemInterface`。
4. 角色已持有 `AbilitySystemComponent`。
5. 角色会在 `BeginPlay` 中初始化 `ActorInfo`。
6. 角色会在 `BeginPlay` 中授予默认战斗 Ability。
7. 普攻输入已具备进入 GAS 的输入桥接链路。
8. 项目内已存在统一战斗 Ability 基类。
9. 项目内已存在最小 Native Gameplay Tags 出口。
10. 项目内当时已存在最小测试普攻 Ability，用于确认 GAS 链路已经打通。

# 本阶段设计边界

1. 本阶段只负责“搭底座”，不负责正式 1-2-3 普攻迁移。
2. 本阶段不实现正式预输入。
3. 本阶段不实现受击、伤害、闪避、格挡和公共战斗语义层。
4. 本阶段不处理完整联机同步。
5. 本阶段测试 Ability 仅用于确认 GAS 链路已打通，不代表正式普通攻击方案已经完成。

# 本阶段核心代码位置

1. 角色接入：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
2. 战斗 Ability 基类：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/`
3. 输入语义定义：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Input/`
4. Gameplay Tag 出口：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/`

# 本阶段实际实现口径

1. `AbilitySystemComponent` 本阶段先挂载在 `AtwoheartsCharacter` 上。
2. 角色自身同时作为 `OwnerActor` 与 `AvatarActor` 初始化 `ActorInfo`。
3. 默认战斗 Ability 授予入口放在角色生成链路。
4. 普攻输入本阶段已不再是纯 Character 本地测试入口，而是能够进入统一输入桥接结构。
5. 当时保留旧普攻最小闭环，是为了给后续正式迁移提供行为对照组。

# 与后续阶段的衔接

1. 本阶段直接后续阶段，是“普通攻击完全切换到 AbilitySystem”。
2. 底座完成后，后续不应再回到 Character 临时状态机继续叠加正式战斗功能。
3. 后续普通攻击、阶段标记、基础打断，都应建立在本阶段提供的 GAS 入口之上继续演进。

# 本阶段历史联调口径

1. 工程编译通过并能识别 GAS 类型。
2. 角色成功持有并返回 `AbilitySystemComponent`。
3. 角色生成时已完成 `ActorInfo` 初始化。
4. 测试 Ability 已在角色生成时被授予。
5. 点击普通攻击输入后，输入成功进入 Ability 激活链路。
6. 当时测试 Ability 激活时只要求打印日志。
7. 当时旧普攻代码仍在工程中保留，不因本阶段底座施工而被破坏。

# 当前与现状不再一致的部分

1. 本文档中提到的 `UTwoHeartsGA_TestNormalAttack` 已不再作为当前正式路径的一部分。
2. 本文档中提到的“旧普攻路径仍可切回”只适用于底座阶段历史状态，不代表当前项目现状。
3. 当前普通攻击正式路径，已经在后续阶段迁入 Gameplay Ability。

# 与当前测试主文档的关系

1. 如果要回看底座是怎么接上的，继续看本文件即可。
2. 如果要进行当前实际联调与测试，请优先阅读：
   [a普通攻击完全切换到AbilitySystem技术文档](./a普通攻击完全切换到AbilitySystem技术文档.md)
3. 当前“GAS 底座测试 + 普攻切换测试”的合并版说明，已经统一收敛到上面这份文档里。
