#文档用途
1.这份文档用于指导资深程序完成第二章第一阶段的直接开发
2.当前目标不是做完整战斗系统，而是先让角色稳定打出最简单的普通攻击连段
3.这是当前程序侧第一份正式开工文档

#基础信息
1.功能名称：最小普通攻击闭环
2.对应设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2.4 节
3.对应技术总文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4.负责实现人：资深程序
5.预计影响模块：角色输入、普通攻击 Ability、动画 Montage、角色基础状态

#实现目标
1.这个功能最终要解决什么问题
   让角色在单角色本地环境下，能够完成最基础的 1-2-3 普通攻击连段，作为第二章后续所有战斗功能的起点。
2.功能的输入、输出分别是什么
   输入：
   玩家普通攻击按键输入；
   策划提供的 3 段普通攻击动画，或 1 个带 3 段 Section 的 Montage。
   输出：
   第 1 段、第 2 段、第 3 段普通攻击播放结果；
   连段推进结果；
   连段结束后回到待机状态。
3.功能生效的前置条件和结束条件是什么
   前置条件：
   角色已有基础输入；
   角色可触发 GAS Ability；
   已有可播放的普通攻击动画资源。
   结束条件：
   点一次普攻播第 1 段；
   连续输入可接出第 2 段和第 3 段；
   不继续输入时正常结束并回待机。

#当前阶段明确不做
1.不做前摇、后摇、有效段、逻辑结束等正式阶段语义
2.不做闪避、格挡、技能、受击对普攻的打断
3.不做通用预输入
4.不做正式命中、伤害、受击反应
5.不做复杂状态 Tag 体系

#模块拆分
1.模块A：普通攻击输入与段序管理
模块职责：
接收普攻输入，决定当前应该进入哪一段普攻，维护最小段序状态。
涉及类/蓝图：
角色战斗输入桥接
普通攻击 Ability 基类
角色基础战斗组件或角色类中的最小普攻状态
对外接口：
请求开始普攻
请求推进下一段普攻
重置当前连段

2.模块B：普通攻击播放模块
模块职责：
按当前段序播放对应动画，并在动画完成后决定是继续下一段还是结束连段。
涉及类/蓝图：
`GA_NormalAttack_01`
`GA_NormalAttack_02`
`GA_NormalAttack_03`
或 1 个普攻主 Ability + 3 段 Montage Section
动画蓝图
Montage 资源
对外接口：
播放当前段动画
接收动画结束事件
通知段序推进或连段结束

#函数设计
1.函数名：TryStartNormalAttack
所属模块：普通攻击输入与段序管理
调用时机：
玩家按下普通攻击键时。
输入参数：
当前角色、当前是否正在普攻、当前普攻段序。
返回内容：
是否成功开始本次普攻；如果成功，返回应播放的目标段序。
函数逻辑：
如果当前不在普攻，则进入第 1 段；
如果当前已在普攻且当前阶段允许推进，则记录下一段请求；
如果当前已在最后一段，则不再向后推进。

2.函数名：PlayNormalAttackSegment
所属模块：普通攻击播放模块
调用时机：
确定要进入某一段普攻时。
输入参数：
段序编号、对应动画资源或 Montage Section。
返回内容：
无。
函数逻辑：
按段序播放对应动画，并把当前角色标记为“正在普通攻击中”。

3.函数名：HandleNormalAttackSegmentFinished
所属模块：普通攻击播放模块
调用时机：
当前段动画播放完成时。
输入参数：
当前段序、是否已记录下一段请求。
返回内容：
无。
函数逻辑：
若存在下一段请求，则进入下一段；
若没有，则结束本次普攻并重置段序状态。

4.函数名：ResetNormalAttackCombo
所属模块：普通攻击输入与段序管理
调用时机：
普攻完整结束或异常中断时。
输入参数：
无。
返回内容：
无。
函数逻辑：
清空当前段序、清空下一段请求、清空“正在普攻中”状态。

#逻辑链条
1.从哪个事件或输入开始触发
   从玩家按下普通攻击键开始。
2.中间依次经过哪些模块、类、函数
   普攻输入 -> `TryStartNormalAttack` -> 确定当前段序 -> `PlayNormalAttackSegment` -> 动画播放完成 -> `HandleNormalAttackSegmentFinished` -> 继续下一段或 `ResetNormalAttackCombo`
3.每一步做了什么判断和处理
   第一击时直接进入第 1 段；
   若角色仍在普攻过程中，则只记录是否要接下一段；
   每段结束时检查是否存在下一段请求；
   有则推进，无则结束。
4.最终如何结束，结果如何返回或表现
   第 3 段结束或中途未继续输入后，角色退出普攻状态并回到待机。

#状态与数据
1.这个功能依赖哪些核心状态
   是否正在普通攻击中
   当前普通攻击段序
   是否已记录下一段请求
2.这些状态由谁创建、谁维护、谁清理
   由普通攻击输入与段序管理模块创建和维护；
   由 `ResetNormalAttackCombo` 统一清理。
3.涉及哪些关键数据结构、配置项或表
   最小普攻段序枚举或整数索引
   3 段普攻动画引用
   可选的 Montage Section 名称配置

#蓝图与C++分工
1.哪些部分必须由C++实现
   段序推进、输入响应、当前普攻状态维护、连段结束重置。
2.哪些部分适合交给蓝图配置或表现层处理
   具体动画资源、Montage、Section、动画表现和过渡。
3.两者之间如何通信
   C++ 决定播哪一段；
   蓝图负责实际播放；
   动画结束后再把结果回传给 C++ 做段序推进或结束。

#风险点
1.可能出现的性能问题
   当前阶段风险很低，主要避免在蓝图里分散维护段序状态。
2.可能出现的联机同步问题
   当前不做联机验收，但段序状态应尽量集中，避免未来同步时状态散落。
3.可能出现的状态错乱问题
   最容易出现的问题是：
   动画播完了但段序没重置；
   下一段请求没有被正确消费；
   普攻状态残留导致角色一直认为自己在攻击中。
4.需要重点关注的边界条件
   快速连点时是否稳定进入 1-2-3；
   第 3 段后是否正确结束；
   中途停止输入是否能正常收招并重置。

#联调与验收
1.实现完成后需要和哪些模块联调
   角色输入
   普攻 Ability
   动画蓝图
   普攻动画资源
2.最少需要验证哪些场景
   点一次普攻只播第 1 段；
   连续点击能稳定播出 1-2-3；
   第 2 段或第 3 段结束后不继续点能正确回待机；
   连段结束后再次点击能重新从第 1 段开始。
3.出现问题时优先检查哪些节点
   普攻输入是否进入 `TryStartNormalAttack`；
   当前段序是否正确记录；
   动画结束事件是否触发；
   `ResetNormalAttackCombo` 是否被调用。

#需要策划先提供的内容
1.3 个普通攻击动画资源，或 1 个带 3 段 Section 的 Montage
2.每一段动画对应的命名和资源引用关系
3.当前阶段默认按固定 3 段普攻处理，不要求派生和变体

#资产配置清单
1.推荐资产方案
   当前第一阶段推荐使用 1 个普通攻击 Montage，内部配置 3 个 Section。
   推荐 Montage 命名：`AM_Melee_NormalAttackCombo`
   推荐 Section 命名：
   `Attack_1`
   `Attack_2`
   `Attack_3`
2.可选源动画资产
   如果动画资源先以 3 个独立 Animation Sequence 提供，推荐命名：
   `AS_Melee_NormalAttack_01`
   `AS_Melee_NormalAttack_02`
   `AS_Melee_NormalAttack_03`
   之后由设计师或动画配置人员把它们组装进 `AM_Melee_NormalAttackCombo`。
3.输入资产
   需要准备一个 Enhanced Input 的 Input Action。
   推荐命名：`IA_NormalAttack`
   用途：玩家按下普通攻击键时触发 C++ 的 `TryStartNormalAttack`。
4.输入映射资产
   需要在当前角色使用的 Input Mapping Context 里加入 `IA_NormalAttack`。
   推荐键位：
   鼠标左键用于 PC 本地验证；
   手柄 Face Button Right 或 Right Trigger 可作为后续补充。
5.角色蓝图配置项
   在角色蓝图中配置：
   `NormalAttackAction` = `IA_NormalAttack`
   `NormalAttackMontage` = `AM_Melee_NormalAttackCombo`
   `NormalAttackSectionNames[0]` = `Attack_1`
   `NormalAttackSectionNames[1]` = `Attack_2`
   `NormalAttackSectionNames[2]` = `Attack_3`
6.当前阶段暂不强制的资产内容
   暂不需要命中特效、攻击判定框、受击动画、伤害数据、打断窗口 Notify。
   动画通知点可以以后补；当前 C++ 会按 Section 时长自动推进或收招。

#当前资产落地记录
1.当前角色
   当前用于最小普通攻击闭环联调的角色为 `ChineseWarrior`。
2.当前普攻 Sequence 资产
   当前已完成重定向并确认可正常预览的 3 段普攻动画为：
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_01`
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_02`
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_03`
3.当前保留但不接入本阶段闭环的普攻参考资产
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_04_Reserved`
   `Chinese_Warrior/Animations/Combat_Attack/AS_Melee_NormalAttack_FullCombo_Ref`
   其中第 4 段与整套参考动画暂不进入本阶段 1-2-3 固定连段逻辑。
4.当前普攻 Montage 资产
   当前已创建：
   `Chinese_Warrior/Animations/Combat_Montage/AM_Melee_NormalAttackCombo`
   当前 Montage 内 Section 命名与顺序为：
   `Attack_1`
   `Attack_2`
   `Attack_3`
   对应关系为：
   `Attack_1` -> `AS_Melee_NormalAttack_01`
   `Attack_2` -> `AS_Melee_NormalAttack_02`
   `Attack_3` -> `AS_Melee_NormalAttack_03`
5.当前已入库的闪避预备资产
   以下资产当前只作为后续闪避开发预备，不接入本次最小普通攻击闭环逻辑：
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_F`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_B`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_L`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_R`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_FL45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_FR45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_BL45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_Dodge_BR45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_F`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_B`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_L`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_R`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_FL45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_FR45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_BL45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRun_BR45`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRunFast_F`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRunFast_L`
   `Chinese_Warrior/Animations/Combat_Dodge/AS_Evade_DodgeToRunFast_R`
6.当前资产使用约束
   本文档对应的普通攻击最小闭环只允许程序依赖以下普攻资产：
   `AM_Melee_NormalAttackCombo`
   `Attack_1`
   `Attack_2`
   `Attack_3`
   不允许在本阶段直接依赖闪避预备资产，也不允许把第 4 段普攻接入当前最小闭环。

#当前代码实现记录
1.本次实现日期
   2026-05-05
2.本次新增或修改的核心代码位置
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
3.本次实际完成的功能点
   在角色类中新增 `NormalAttackAction`，用于接收 Enhanced Input 的普通攻击输入。
   在角色类中新增 `NormalAttackMontage` 和 `NormalAttackSectionNames`，用于由蓝图配置 1 个 Montage 与 3 个 Section。
   在角色类中集中维护最小普攻状态：`bIsNormalAttacking`、`CurrentNormalAttackSegment`、`bHasQueuedNextNormalAttackSegment`。
   实现 `TryStartNormalAttack`：角色不在普攻中时进入第 1 段；角色正在普攻且未到第 3 段时记录下一段请求；第 3 段时不再继续推进。
   实现 `PlayNormalAttackSegment`：校验段序、Montage、AnimInstance、Section 名称与 Section 时长后播放对应 Section，并用 Timer 等待当前段结束。
   实现 `HandleNormalAttackSegmentFinished`：当前段结束时，如果存在下一段请求则进入下一段，否则调用 `ResetNormalAttackCombo` 收招。
   实现 `ResetNormalAttackCombo`：清理 Timer、停止当前普攻 Montage、重置段序和缓存状态。
4.本次蓝图配置入口
   `NormalAttackAction`：配置为 `IA_NormalAttack`。
   `NormalAttackMontage`：配置为 `AM_Melee_NormalAttackCombo`。
   `NormalAttackSectionNames`：默认已经填入 `Attack_1`、`Attack_2`、`Attack_3`，蓝图中可按实际 Section 名称覆盖。
5.与原计划不一致的地方
   技术总文档提到“普通攻击每一段按独立 Ability 组织”，但当前项目代码中尚未接入 GAS Ability 基础类与 AbilitySystemComponent。
   为了先完成本地最小闭环，本次把段序状态和播放逻辑暂时集中在 `AtwoheartsCharacter` 中；后续接入 GAS 时，可把这四个核心函数迁移到普通攻击 Ability 或角色战斗组件。
6.当前保护策略
   未配置 `NormalAttackAction` 时不会绑定普攻输入。
   未配置 `NormalAttackMontage`、缺少 AnimInstance、Section 名称不存在、Section 时长异常或 Montage 播放失败时，会打印 Warning 并安全重置，不会因为资产缺失导致编译失败。
7.当前遗留问题
   还没有正式接入 GAS Ability。
   还没有命中、伤害、受击、打断、预输入窗口和动画 Notify。
   当前依赖 Section 时长推进段落，后续阶段标记文档落地后，应改为动画通知或动作逻辑事件驱动。
8.本次验证结果
   已执行 `twoheartsEditor Win64 Development` 编译，结果成功。
   已完成 `ChineseWarrior` 普攻动画重定向与编辑器内预览检查，当前 `AS_Melee_NormalAttack_01~03` 与 `AM_Melee_NormalAttackCombo` 预览结果正常。
   已完成 `AM_Melee_NormalAttackCombo` 的 3 段 Section 配置：`Attack_1`、`Attack_2`、`Attack_3`。
   尚未完成编辑器内完整输入联调，仍需等待 `IA_NormalAttack`、Input Mapping Context 和角色蓝图配置全部接好后验证实际 1-2-3 连段。

#本阶段交付结果
1.角色具备最小普通攻击能力
2.普通攻击连段可稳定推进和重置
3.后续“阶段标记”“打断”“预输入”都将在此基础上继续扩展
