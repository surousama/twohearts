#文档用途
1.这份文档用于指导资深程序完成《双心印》第二章当前下一阶段的直接开发。
2.当前目标不是继续叠加玩法功能，而是先把战斗系统的正式 `AbilitySystem` 基础底座搭起来。
3.这是当前程序侧新的直接开工文档。

#基础信息
1.功能名称：AbilitySystem 基础底座
2.对应技术总文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
3.负责实现人：资深程序
4.当前验收口径：单角色本地验证
5.长期目标：为普通攻击、阶段标记、预输入、受击、闪避、格挡提供正式承载基础

#为什么现在先做这个
1.技术总文档已经明确第二章长期路线是 `UE5 + GAS + C++`。
2.当前最小普通攻击闭环虽然已完成，但实现仍暂放在 `AtwoheartsCharacter` 中，只是过渡方案。
3.当前源码里还没有正式的 `AbilitySystemComponent`、`GameplayAbility`、`GameplayTag` 基础代码。
4.如果现在直接在临时 Character 状态机上继续做预输入、打断和阶段语义，后续接入 GAS 时会产生明显返工。

#当前项目现状
1.当前最小普攻闭环已完成，相关代码位置：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
2.当前普通攻击承载位置：
   `AtwoheartsCharacter`
3.当前 `twohearts.Build.cs` 尚未加入：
   `GameplayAbilities`
   `GameplayTags`
   `GameplayTasks`
4.当前源码目录下尚未发现：
   通用角色战斗组件
   ASC 挂载代码
   基础战斗 Ability 基类
   基础战斗 Tag 注册代码

#本阶段实现目标
1.让角色正式具备 `AbilitySystemComponent` 接入能力。
2.建立第二章后续可复用的基础战斗 Ability 基类。
3.建立最小可用的基础战斗 Tag 注册与访问方式。
4.建立角色输入到 Ability 的基础桥接路径。
5.为下一阶段“普通攻击接入 AbilitySystem”提供稳定入口。

#本阶段明确不做
1.不在本阶段重做完整普通攻击逻辑。
2.不在本阶段直接实现正式预输入。
3.不在本阶段直接实现闪避、格挡、受击和伤害。
4.不在本阶段直接实现完整公共战斗语义层。
5.不在本阶段追求联机完整同步。

#建议模块拆分
1.模块A：Build 依赖与工程底座
模块职责：
补齐 GAS 所需模块依赖，确保工程层可以编译和引用 AbilitySystem 相关类型。
建议落点：
`twohearts/Source/twohearts/twohearts.Build.cs`

2.模块B：角色 ASC 接入
模块职责：
让当前主角基类或战斗角色基类正式持有并暴露 `AbilitySystemComponent`。
建议落点：
角色基类头源文件；
如后续需要，也可拆出角色战斗组件。

3.模块C：基础战斗 Ability 基类
模块职责：
提供第二章战斗 Ability 的统一基类，先统一基础引用、Owner 获取、Avatar 获取、基础日志和常用工具接口。
建议落点：
`Source/twohearts/Variant_Combat/Gameplay/Abilities/` 或当前项目约定的战斗 Gameplay 目录。

4.模块D：基础战斗 Tag
模块职责：
先建立第二章后续必需的最小 Tag 注册方式和命名出口，避免后面各模块自行发明命名。
建议落点：
战斗 Tag 定义头源文件；
后续可与公共语义层文档中的 Tag 结构对齐。

5.模块E：输入到 Ability 的桥接
模块职责：
建立普通攻击输入从 Enhanced Input 进入 Ability 激活链路的最小桥接。
建议落点：
角色输入绑定层；
必要时增加简单输入枚举或 Input Tag 对应表。

#建议类与接口
1.角色侧最小要求
   角色应实现 `IAbilitySystemInterface` 或提供等价稳定访问方式；
   角色应暴露 `GetAbilitySystemComponent`；
   角色应能在合适时机初始化 ActorInfo。
2.ASC 侧最小要求
   角色拥有可访问的 `UAbilitySystemComponent`；
   当前阶段不要求复杂扩展，但应保留后续继承空间。
3.Ability 基类最小要求
   提供统一战斗 Ability 基类，例如 `UTwoHeartsGameplayAbility` 或项目内等价命名；
   后续普通攻击、闪避、格挡都从该基类派生。
4.Tag 侧最小要求
   至少建立基础战斗类别命名出口；
   当前先保证命名统一，不要求一次性把第二章全部 Tag 注册完。

#建议最小 Tag 范围
1.动作类预留
   `State.Action.NormalAttack`
   `State.Action.Dodge`
   `State.Action.Guard`
2.限制类预留
   `State.CannotAttack`
   `State.CannotInput`
3.能力类预留
   `Ability.NormalAttack`
4.说明
   当前只做最小底座需要的第一批；
   后续阶段标记与公共语义层再继续补充。

#逻辑链条
1.从哪个事件开始
   从角色创建、角色被玩家控制、输入绑定和 Ability 授予开始。
2.中间依次经过哪些模块
   工程模块依赖补齐 -> 角色接入 ASC -> 初始化 ActorInfo -> 授予最小战斗 Ability -> 输入桥接 Ability 激活。
3.本阶段最终结果
   角色侧已经具备正式 GAS 入口；
   后续普通攻击迁移时不再需要把战斗逻辑直接写死在 `AtwoheartsCharacter`。

#与当前普攻闭环的关系
1.当前最小普攻闭环不要直接删除。
2.本阶段完成后，应把它视为迁移参照物。
3.下一阶段应在 AbilitySystem 底座上复现当前 1-2-3 行为结果，再逐步移除 Character 内的临时战斗状态。

#施工注意事项
1.当前项目已有普攻最小闭环和调试能力，施工时不要破坏现有本地验证路径。
2.本阶段重点是“搭底座”，不要顺手把预输入、打断、受击等功能一起揉进来。
3.当前若没有完整战斗组件，也可以先在角色基类接入 ASC，但代码结构要为后续迁移预留空间。
4.当前若暂不采用“每段普攻一个独立 Ability”的完整拆法，也至少要先建立统一战斗 Ability 基类。
5.当前不建议把阶段语义硬编码进 AbilitySystem 底座文档；阶段语义应在后续“普攻阶段标记与基础打断”阶段落实。

#风险点
1.若只加模块依赖，但不把角色初始化链路打通，后续 Ability 看似可编译，实际无法稳定激活。
2.若现在不统一基础 Ability 基类，后续普通攻击、闪避、格挡会各自长出不同接法。
3.若现在不建立最小 Tag 命名出口，后续公共语义层会被迫清理历史命名债务。
4.若本阶段把普攻逻辑和底座接入混在一起做，排错成本会明显上升。

#联调与验收
1.实现完成后需要联调的内容
   角色创建链路
   输入绑定链路
   Ability 授予链路
   Ability 激活链路
2.最少需要验证的场景
   工程可成功编译并识别 GAS 相关类型；
   角色成功持有并返回 ASC；
   角色可被授予一个最小测试 Ability；
   普攻输入可以走到 Ability 激活入口；
   当前最小普攻闭环仍可作为旧路径保留，直到迁移完成。
3.出现问题时优先检查的节点
   `twohearts.Build.cs` 模块依赖是否补齐；
   角色是否正确初始化 ASC；
   `ActorInfo` 是否在正确时机初始化；
   输入是否真正走到 Ability 激活入口；
   测试 Ability 是否被实际授予给角色。

#建议交付结果
1.补齐 `GameplayAbilities`、`GameplayTags`、`GameplayTasks` 模块依赖。
2.角色具备稳定可访问的 ASC。
3.项目内具备统一战斗 Ability 基类。
4.项目内具备最小战斗 Tag 命名与访问出口。
5.普通攻击 Ability 接入所需的输入桥接路径已经打通。

#本阶段完成后建议衔接的下一步
1.编写或补充“普通攻击接入 AbilitySystem”实施文档。
2.在 GAS 底座上复现当前 1-2-3 普攻最小闭环。
3.随后进入“普通攻击阶段标记与基础打断”。
