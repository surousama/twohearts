#文档用途
1.这份文档用于指导《双心印》第二章当前阶段“AbilitySystem 基础底座”的具体实现落地。
2.本文档是资深程序维护的独立技术实现文档，不替代上游技术总文档与开工文档。
3.本文档用于明确代码目录、类职责、初始化链路、输入桥接、Tag 注册、默认 Ability 授予与旧普攻共存方案。

#基础信息
1.功能名称：AbilitySystem 基础底座
2.上游开工文档：[aAbilitySystem基础底座技术文档](./aAbilitySystem基础底座技术文档.md)
3.上游总文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4.对应设计框架：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2 章
5.当前验收口径：单角色本地验证
6.当前阶段目标：搭建正式 GAS 入口，不重做完整普通攻击逻辑

#本阶段最终确认口径
1.`AbilitySystemComponent` 本阶段先挂载在 `AtwoheartsCharacter` 上。
2.代码结构需要为后续提炼战斗角色基类或战斗组件预留迁移空间。
3.默认 Ability 在角色生成时授予，玩家与敌人未来尽量统一这条初始化思路。
4.输入桥接不做临时直连方案，直接按长期方案落地。
5.战斗 Tag 从本阶段开始统一注册与统一出口，不允许后续各模块零散命名。
6.测试 Ability 的本阶段验收表现仅要求打印日志。
7.当前旧普攻闭环继续保留，本阶段目标是“新 GAS 入口可用，旧普攻照常可跑”。
8.后续普通攻击长期结构按“每一段普通攻击一个独立 Ability”预留。

#总体实现目标
1.让当前角色正式持有并暴露 `AbilitySystemComponent`。
2.建立战斗 Ability 的统一基类，作为第二章后续普通攻击、闪避、格挡、受击的共同父类。
3.建立最小 Native Gameplay Tags 出口，统一第二章战斗语义命名。
4.建立从输入语义到 Ability 激活的统一桥接结构。
5.建立默认战斗 Ability 授予入口，保证角色生成后具备正式 GAS 初始化链路。
6.在不破坏当前旧普攻本地验证路径的前提下，为下一阶段“普通攻击迁入 AbilitySystem”打底。

#明确不做
1.不在本阶段把 1-2-3 普攻完整迁入 GAS。
2.不在本阶段实现正式预输入。
3.不在本阶段实现受击、闪避、格挡、伤害与公共战斗语义层。
4.不在本阶段解决完整联机同步。
5.不在本阶段实现正式动画通知驱动的阶段系统。

#推荐目录结构
1.角色接入继续放在：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
2.Build 依赖落点：
   `twohearts/Source/twohearts/twohearts.Build.cs`
3.战斗 Ability 基类目录：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/`
4.战斗 Tag 定义目录：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/`
5.输入桥接与输入配置目录：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Input/`

#为什么本阶段先挂在 AtwoheartsCharacter
1.当前项目里已经明确存在并正在承担角色控制职责的类是 `AtwoheartsCharacter`，直接接入风险最低。
2.当前敌人体系和长期战斗角色基类尚未定型，如果现在强拆新的角色父类，容易先造出一层空壳。
3.本阶段真正需要稳定的是 `ASC`、战斗 Ability 基类、Tag、输入桥接和默认授予流程，而不是提前制造角色继承层级。
4.只要角色上层只承担接入与初始化职责，后续无论迁到 `ACombatCharacterBase` 还是战斗组件，返工都会相对可控。

#核心类规划
1.角色侧
   `AtwoheartsCharacter`
   职责：
   持有 `AbilitySystemComponent`；
   实现 `IAbilitySystemInterface`；
   在角色生成时初始化 `ActorInfo`；
   配置默认授予的战斗 Ability；
   处理输入桥接入口。
2.ASC 侧
   本阶段先直接使用 `UAbilitySystemComponent`；
   不急着自定义派生类；
   但角色成员命名、访问接口和初始化流程应预留未来替换为 `UTwoHeartsAbilitySystemComponent` 的空间。
3.基础战斗 Ability 基类
   建议命名：
   `UTwoHeartsGameplayAbility`
   职责：
   作为第二章所有战斗 Ability 的统一父类；
   提供 Owner、Avatar、Character、ASC 的便捷访问；
   提供统一日志输出能力；
   提供默认激活策略和常用帮助函数。
4.基础测试 Ability
   建议命名：
   `UTwoHeartsGA_TestNormalAttack`
   职责：
   仅用于验证输入桥接与 Ability 激活链路；
   激活成功后打印日志；
   本阶段不承载正式 1-2-3 普攻逻辑。
5.Tag 出口
   建议命名：
   `FTwoHeartsGameplayTags`
   职责：
   集中注册战斗相关 Native Gameplay Tags；
   提供全局统一访问入口；
   作为后续公共语义层和各 Ability 的唯一命名来源。
6.输入配置
   建议命名：
   `ETwoHeartsAbilityInputID`
   或等价输入枚举 / 输入语义定义结构。
   职责：
   统一输入语义；
   让 Ability 授予和输入激活共享同一套稳定编号或 Tag。

#输入桥接方案
1.本阶段不采用“角色收到输入后手动直接调用测试函数”的临时做法。
2.本阶段直接建立长期桥接结构：输入动作先映射到统一战斗输入语义，再进入 Ability 激活。
3.普通攻击输入当前建议流程：
   `NormalAttackAction`
   ->
   `AtwoheartsCharacter` 输入绑定函数
   ->
   统一战斗输入语义
   ->
   `AbilitySystemComponent` 输入触发
   ->
   测试 Ability 激活
4.本阶段只需要先接通普通攻击一条输入桥接链路。
5.后续闪避、格挡、技能、结印可以沿用同一桥接方式继续扩展。

#Tag 注册方案
1.本阶段采用 Native Gameplay Tags 统一注册方案。
2.当前最小注册范围如下：
   `Ability.NormalAttack`
   `State.Action.NormalAttack`
   `State.Action.Dodge`
   `State.Action.Guard`
   `State.CannotAttack`
   `State.CannotInput`
3.本阶段 Tag 的主要用途：
   标识 Ability 类型；
   标识动作占用状态；
   标识输入限制状态；
   为后续打断关系和公共语义层预留命名基础。
4.本阶段不一次性铺满第二章全部 Tag，只做第一批稳定最小集合。
5.后续新 Tag 必须在同一出口继续追加，不能零散写死在 Ability 内部。

#默认 Ability 授予方案
1.默认战斗 Ability 的授予入口放在角色生成链路。
2.当前阶段建议由角色配置一个“默认战斗 Ability 列表”。
3.角色生成后完成如下步骤：
   创建并持有 `ASC`
   ->
   初始化 `ActorInfo`
   ->
   授予默认战斗 Ability
   ->
   允许输入桥接触发激活
4.本阶段只要求最小测试 Ability 成功被授予并可激活。
5.后续普通攻击每段 Ability、闪避 Ability、格挡 Ability 都可以沿用同一默认授予入口。

#ActorInfo 初始化方案
1.本阶段采用角色自身作为 OwnerActor 与 AvatarActor 的最小方案。
2.初始化时机按“角色生成时”处理，以满足玩家与敌人未来统一。
3.实现时需要确保：
   `ASC` 创建完成后可稳定返回；
   `InitAbilityActorInfo` 在授予 Ability 前已执行；
   本地单角色场景下不会因为初始化顺序导致 Ability 无法激活。
4.如果后续项目引入 `PlayerState` 持有 ASC，再重新评估 OwnerActor 与 AvatarActor 分离方案。

#旧普攻共存方案
1.当前 `AtwoheartsCharacter` 内已有的 1-2-3 普攻逻辑本阶段不删除。
2.旧普攻当前继续作为迁移参考和回归对照组保留。
3.本阶段新接入的测试 Ability 只验证 GAS 链路，不直接替换旧普攻行为。
4.下一阶段“普通攻击接入 AbilitySystem”时，再决定普通攻击输入最终切换到哪条路径。
5.本阶段需要避免把旧普攻临时状态机和新 GAS 底座强行混写到一起。

#与后续“每段普攻一个 Ability”结构的衔接
1.本阶段虽然只做测试 Ability，但命名和结构要按长期方案预留。
2.后续普通攻击迁移时，建议至少预留以下形态：
   `UTwoHeartsGA_NormalAttackBase`
   `UTwoHeartsGA_NormalAttack_1`
   `UTwoHeartsGA_NormalAttack_2`
   `UTwoHeartsGA_NormalAttack_3`
3.其中：
   `NormalAttackBase` 负责共享普通攻击通用能力；
   每一段独立 Ability 负责该段动画、Tag、命中与后续衔接策略。
4.本阶段的测试 Ability 不应占用最终正式类名，避免后续重命名噪音。

#建议代码职责边界
1.角色类负责：
   `ASC` 生命周期持有；
   输入绑定；
   默认 Ability 授予入口；
   最小初始化调度。
2.基础战斗 Ability 基类负责：
   常用对象访问；
   日志；
   通用辅助接口；
   未来公共默认行为。
3.Tag 模块负责：
   集中定义与注册；
   对外统一只读访问。
4.输入桥接模块负责：
   输入语义定义；
   输入与 Ability 的映射关系。
5.旧普攻逻辑继续局部留在 Character，直到下一阶段迁移完成。

#最小联调链路
1.工程编译通过并能识别 GAS 类型。
2.角色成功持有并返回 `AbilitySystemComponent`。
3.角色生成时已完成 `ActorInfo` 初始化。
4.测试 Ability 已在角色生成时被授予。
5.点击普通攻击输入后，输入成功进入 Ability 激活链路。
6.测试 Ability 激活时打印日志。
7.旧普攻代码仍在工程中保留，不因本阶段底座施工而被破坏。

#验收标准
1. `twohearts.Build.cs` 已补齐 `GameplayAbilities`、`GameplayTags`、`GameplayTasks` 依赖。
2. `AtwoheartsCharacter` 已具备稳定可访问的 `ASC`。
3. 项目内存在统一的战斗 Ability 基类。
4. 项目内存在统一的 Native Gameplay Tags 出口。
5. 普攻输入可以进入正式 GAS 输入桥接链路。
6. 测试 Ability 可成功激活并打印日志。
7. 当前旧普攻闭环仍保留为迁移参照。

#风险与注意事项
1.如果只补了模块依赖，但没有打通 `ActorInfo` 初始化和授予顺序，Ability 仍会处于“能编译、不能稳定激活”的假接入状态。
2.如果输入桥接仍采用临时手动直连，后续闪避、格挡、技能接入时会再次返工。
3.如果本阶段不用统一 Tag 出口，后续公共语义层会积累命名债务。
4.如果现在把测试 Ability 与正式普攻迁移混做，排错会明显变难。
5.如果现在把大量战斗状态继续写死在 Character 里，后续再抽结构时成本会上升。

#本阶段完成后的下一步建议
1.进入代码实现，严格按本文档落地 AbilitySystem 基础底座。
2.实现完成后补充实际代码文件与函数清单。
3.随后编写“普通攻击接入 AbilitySystem”实施文档。
4.在 GAS 底座上复现当前 1-2-3 普攻最小闭环。

#当前已落地实现记录
1.实现日期：
   2026-05-14
2.本次已完成内容：
   `twohearts.Build.cs` 已补齐 `GameplayAbilities`、`GameplayTags`、`GameplayTasks` 依赖；
   `twohearts.uproject` 已启用 `GameplayAbilities` 插件；
   `AtwoheartsCharacter` 已实现 `IAbilitySystemInterface`；
   角色已持有 `AbilitySystemComponent`；
   角色会在 `BeginPlay` 中初始化 `ActorInfo`；
   角色会在 `BeginPlay` 中授予默认战斗 Ability；
   普攻输入已具备进入 GAS 的输入桥接链路；
   项目内已存在统一战斗 Ability 基类；
   项目内已存在最小 Native Gameplay Tags 出口；
   项目内已存在最小测试普攻 Ability。
3.当前新增代码目录：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Input/`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/`
4.当前最小测试 Ability：
   `UTwoHeartsGA_TestNormalAttack`
5.当前最小测试 Ability 的验收表现：
   激活时打印日志；
   不播放正式普攻；
   不接管 1-2-3 连段；
   只用于确认 GAS 链路已经打通。
6.当前编译结果：
   已使用 `H:\UE_5.6\Engine\Build\BatchFiles\Build.bat` 编译 `twoheartsEditor Win64 Development`；
   结果为成功。

#策划与联调注意事项
1.当前普通攻击输入默认先走 GAS。
2.当前控制这个行为的开关是：
   `bUseAbilitySystemForNormalAttackInput`
3.当前开关代码位置：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
4.当前开关默认值：
   `true`
5.当前开关含义：
   为 `true` 时，普通攻击输入会优先进入 GAS 输入桥接，并尝试激活测试 Ability；
   为 `false` 时，普通攻击输入会回到旧的 `TryStartNormalAttack` 路径。
6.当前阶段策划联调时，如果发现“按下普攻没有播放原 1-2-3 连段”，这不一定是 Bug。
7.原因是：
   当前默认验收目标是“测试 Ability 打印日志”，不是“正式普攻已经迁移完成”。
8.如果当前需要继续回看旧普攻 1-2-3 表现，有两种方式：
   方式一：把 `bUseAbilitySystemForNormalAttackInput` 临时改为 `false`；
   方式二：后续在蓝图默认值暴露后，从角色蓝图切回旧路径联调。
9.当前 `bUseAbilitySystemForNormalAttackInput` 还只是 C++ 属性，后续如果策划需要频繁切换，建议下一阶段把它稳定暴露到角色蓝图默认值检查流程中。
10.当前默认授予列表是：
    `DefaultCombatAbilities`
11.当前默认授予列表代码位置：
    `twohearts/Source/twohearts/twoheartsCharacter.h`
12.当前默认授予逻辑代码位置：
    `twohearts/Source/twohearts/twoheartsCharacter.cpp`
13.当前阶段如果后续要增加“闪避测试 Ability”“格挡测试 Ability”，建议继续走 `DefaultCombatAbilities`，不要再单独写死新入口。
14.当前 Ability 初始化时机是：
    `BeginPlay`
15.当前实现口径是“角色生成后初始化并授予”，与上游已确认方向一致。
16.当前测试 Ability 名称是：
    `UTwoHeartsGA_TestNormalAttack`
17.当前测试 Ability 代码位置：
    `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_TestNormalAttack.h`
    `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_TestNormalAttack.cpp`
18.当前测试 Ability 只负责打印日志，不代表正式普通攻击方案已经完成。
19.当前阶段策划最需要避免的误判是：
    不要把“GAS 测试 Ability 成功打印日志”理解为“普通攻击迁移已完成”；
    也不要把“没有播放旧普攻动画”直接理解为“普攻系统失效”。
20.当前阶段正确的验收重点应是：
    角色是否成功持有 ASC；
    角色生成时是否完成初始化；
    普攻输入是否走到 Ability 激活链路；
    测试 Ability 是否成功打印日志；
    旧普攻路径是否仍可保留切回。

#后续衔接：普通攻击接入 AbilitySystem
1.这部分只作为当前文档中的后续施工提示，不替代后面主程序正式整理的技术文档。
2.下一阶段目标：
   在当前已经打通的 GAS 底座上，复现现有 1-2-3 普攻最小闭环。
3.下一阶段需要做的核心内容：
   把当前 Character 内的普攻承载逻辑迁入正式 Ability；
   按长期方向预留“一段普攻一个 Ability”的结构；
   保持当前 1-2-3 输入、段序推进、结束回待机的行为结果不变；
   保留旧 Character 版本作为迁移对照，直到新链路稳定。
4.建议最小实施顺序：
   先新增普通攻击 Ability 基类；
   再补第 1 段、第 2 段、第 3 段普通攻击 Ability；
   再把普通攻击输入从测试 Ability 切到正式普通攻击入口；
   最后对齐旧普攻闭环表现并做回归验证。
5.下一阶段迁移时，当前 `UTwoHeartsGA_TestNormalAttack` 不再承担正式普攻职责，应让它退出正式普攻入口。
6.下一阶段应优先迁移的内容：
   普攻输入响应；
   段序切换；
   连段结束重置；
   基础日志与调试信息。
7.下一阶段暂时仍不建议一起揉进去的内容：
   正式预输入；
   阶段标记；
   打断关系；
   受击与伤害；
   闪避和格挡联动。
8.下一阶段验收口径建议保持简单：
   普攻输入已不再走测试 Ability；
   1-2-3 普攻可在 AbilitySystem 上复现；
   行为结果与旧普攻最小闭环一致；
   旧路径仍可作为对照保留到迁移确认完成。
