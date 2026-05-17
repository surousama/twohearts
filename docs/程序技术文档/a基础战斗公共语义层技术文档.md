# 文档用途

1. 这份文档用于资深程序记录《双心印》第二章“基础战斗模块”中“公共战斗语义层”的具体技术实现方案
2. 这不是设计文档，而是面向开发、联调、排查和交接的技术文档
3. 当前文档由主程序视角输出，用于给资深程序明确底层动作语义框架的落地方式
4. 当前文档不是第一阶段开工文档，而是后续在多个子系统跑通后再正式收束共通规则时使用

# 基础信息

1. 功能名称：基础战斗公共语义层
2. 对应设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2 章
3. 对应主文档：[a第二章基础战斗模块开发总文档](./a第二章基础战斗模块开发总文档.md)
4. 负责实现人：资深程序
5. 预计影响模块：角色输入、普通攻击、闪避、格挡、受击、后续结印系统、动画蒙太奇事件、GAS Tag 体系

# 实现目标

1. 这个功能最终要解决什么问题
   把第二章里反复出现的“动作阶段”“打断关系”“输入权限”“状态标签”“动作逻辑结束事件”统一收口，避免普通攻击、闪避、格挡、受击、后续结印各自维护一套互相冲突的战斗规则。
2. 功能的输入、输出分别是什么
   输入：
   玩家输入请求、当前动作上下文、角色身上的战斗标签、当前 Ability 状态、动画或逻辑事件。
   输出：
   动作是否允许执行、是否允许进入预输入、当前动作阶段、动作打断结果、动作逻辑结束事件、对外可读的统一战斗状态。
3. 功能生效的前置条件和结束条件是什么
   前置条件：
   角色已接入 ASC，具备基础战斗 Tag，Ability 可向公共语义层上报动作开始、阶段切换和动作结束。
   结束条件：
   普通攻击、闪避、格挡、受击都通过统一接口查询和更新动作语义，不再在各 Ability 内各自硬编码动作阶段与打断判断。

# 模块拆分

1. 模块A：动作上下文管理模块
模块职责：
统一记录角色当前正在执行的战斗动作、动作来源、动作阶段、是否允许输入、是否允许被打断、是否允许进入预输入。
涉及类/蓝图：
`UCombatActionComponent`
`FCombatActionContext`
角色基类或角色战斗组件挂载点
对外接口：
`BeginCombatAction`
`UpdateCombatActionPhase`
`EndCombatAction`
`GetCurrentCombatActionContext`
`IsInCombatAction`

2. 模块B：打断规则判定模块
模块职责：
统一维护“谁能打断谁”“可打断前摇还是后摇”“不可打断特例”等规则，并向所有主动动作提供统一判断接口。
涉及类/蓝图：
`FCombatInterruptRule`
`UCombatActionComponent`
战斗 Tag 定义表或数据资产
对外接口：
`CanActionInterruptCurrent`
`CanCurrentActionBeInterruptedBy`
`ResolveInterruptRule`

3. 模块C：输入权限与预输入接入模块
模块职责：
统一判断一个输入是应当立即执行、进入预输入，还是直接拒绝。
涉及类/蓝图：
`FCombatInputRequest`
`ECombatInputType`
`UCombatActionComponent`
对外接口：
`EvaluateCombatInputRequest`
`CanInputEnterBuffer`
`ShouldExecuteInputImmediately`

4. 模块D：动作逻辑结束事件模块
模块职责：
把“动作结束”从纯动画概念中解耦出来，统一管理逻辑结束时机，并向预输入、状态清理、后续动作衔接广播标准事件。
涉及类/蓝图：
`FCombatActionEndReason`
`UCombatActionComponent`
Ability 基类中的事件桥接
对外接口：
`NotifyCombatActionLogicEnd`
`BroadcastCombatActionEnded`
`BindOnCombatActionLogicEnd`

5. 模块E：公共战斗标签定义模块
模块职责：
收敛第二章基础战斗相关的公共 Tag 命名和分类，避免各系统自行发明标签。
涉及类/蓝图：
基础战斗 Tag 清单
Tag 注册代码
配置文档
对外接口：
提供统一 Tag 常量访问，不提供独立运行逻辑。

# 函数设计

1. 函数名：BeginCombatAction
所属模块：动作上下文管理模块
调用时机：
任意主动战斗动作 Ability 成功开始时。
输入参数：
动作类型、动作名、来源 Ability、初始动作阶段、是否允许预输入、是否允许被打断、动作相关 Tag。
返回内容：
是否成功进入动作上下文。
函数逻辑：
检查是否已有不可覆盖的当前动作；若可进入则写入新的 `FCombatActionContext`，设置初始阶段、基础权限和动作标签，并广播动作开始事件。

2. 函数名：UpdateCombatActionPhase
所属模块：动作上下文管理模块
调用时机：
动作从前摇进入有效段、从有效段进入后摇、从后摇进入逻辑结束前等阶段时。
输入参数：
目标阶段、新阶段附加标签、阶段时间戳。
返回内容：
是否更新成功。
函数逻辑：
校验调用者是否仍为当前动作拥有者；成功后更新当前动作阶段，并刷新“是否允许输入”“是否允许被打断”等派生状态。

3. 函数名：EvaluateCombatInputRequest
所属模块：输入权限与预输入接入模块
调用时机：
玩家产生战斗输入时。
输入参数：
输入类型、目标动作类型、角色当前上下文、当前 Tag 状态、资源快照。
返回内容：
输入评估结果：
立即执行、进入预输入、拒绝执行。
函数逻辑：
先检查限制标签和基础资源条件，再检查当前是否存在动作上下文；若无当前动作则直接执行；若有当前动作则交给打断规则模块判断，能打断则立即执行，不能打断但允许缓存则进入预输入，否则拒绝。

4. 函数名：ResolveInterruptRule
所属模块：打断规则判定模块
调用时机：
新动作尝试打断当前动作时。
输入参数：
当前动作上下文、新输入类型、新动作标签、当前角色 Tag。
返回内容：
是否允许打断，以及原因。
函数逻辑：
优先检查当前动作是否显式不可打断，再检查新动作是否拥有强制打断资格，再检查当前阶段是否落在可被打断区间，最后结合特例 Tag 给出结果。

5. 函数名：NotifyCombatActionLogicEnd
所属模块：动作逻辑结束事件模块
调用时机：
动作逻辑已经结束，但动画可能仍在收招表现时。
输入参数：
动作拥有者、结束原因、是否允许立刻衔接下一动作。
返回内容：
无。
函数逻辑：
确认当前上下文仍有效后，把当前动作标记为逻辑结束，广播标准化结束事件，通知预输入模块结算，并进入统一清理流程。

6. 函数名：EndCombatAction
所属模块：动作上下文管理模块
调用时机：
动作完全结束，或者被受击/强制行为打断后需要清理当前上下文时。
输入参数：
结束原因、是否清空阶段标签、是否强制清空缓存。
返回内容：
无。
函数逻辑：
清理当前 `FCombatActionContext`，移除本动作写入的阶段性状态，必要时广播中断事件，并根据参数决定是否要求预输入模块清空缓存。

# 逻辑链条

1. 从哪个事件或输入开始触发
   从玩家主动输入、Ability 启动、动画通知阶段切换、受击打断、闪避取消、格挡成功/失败结束等事件触发。
2. 中间依次经过哪些模块、类、函数
   玩家输入 -> `EvaluateCombatInputRequest` -> 若立即执行则 Ability 开始 -> `BeginCombatAction` -> 动画或逻辑阶段切换时调用 `UpdateCombatActionPhase` -> 动作逻辑结束时 `NotifyCombatActionLogicEnd` -> 完全结束时 `EndCombatAction`
3. 每一步做了什么判断和处理
   输入阶段：
   判断限制条件、动作是否可执行、是否可打断当前动作、是否允许进入预输入。
   动作阶段：
   当前动作必须把自身处于前摇、有效段、后摇、逻辑结束等待、完全结束中的哪一段，统一写回语义层。
   结束阶段：
   逻辑结束先通知预输入和后续衔接，再由真正的 Ability End 或强制打断做最终清理。
4. 最终如何结束，结果如何返回或表现
   结果通过三层返回：
   给程序的是统一动作上下文和评估结果；
   给其他系统的是标准化结束/中断事件；
   给蓝图表现层的是可监听的动作开始、阶段变化、逻辑结束、完全结束回调。

# 状态与数据

1. 这个功能依赖哪些核心状态
   当前动作类型
   当前动作唯一标识
   当前动作阶段
   当前动作拥有者 Ability
   当前动作是否允许被打断
   当前动作是否允许进入预输入
   当前动作是否已逻辑结束
   当前动作开始时间和阶段切换时间
   角色当前基础限制 Tag
2. 这些状态由谁创建、谁维护、谁清理
   `FCombatActionContext` 由 `UCombatActionComponent` 创建和维护；
   Ability 在开始和阶段切换时主动上报；
   逻辑结束由动作自身主动通知；
   最终清理由 `EndCombatAction` 收口，禁止各系统各自偷偷移除关键状态。
3. 涉及哪些关键数据结构、配置项或表
   `enum class ECombatActionType`
   `enum class ECombatActionPhase`
   `enum class ECombatInputEvaluationResult`
   `enum class ECombatActionEndReason`
   `struct FCombatActionContext`
   `struct FCombatInterruptRule`
   `struct FCombatInputRequest`
   基础战斗 Tag 清单

# 建议数据定义

1. 动作类型 `ECombatActionType`
   至少包含：
   None
   NormalAttack
   Skill
   Dodge
   Guard
   HitReaction
   ScriptedMove
2. 动作阶段 `ECombatActionPhase`
   至少包含：
   None
   Startup
   Active
   Recovery
   LogicEnded
   Finished
3. 动作结束原因 `ECombatActionEndReason`
   至少包含：
   NaturalEnd
   InterruptedByHit
   InterruptedByDodge
   InterruptedBySkill
   InterruptedByGuard
   ForcedCancel
4. 输入评估结果 `ECombatInputEvaluationResult`
   至少包含：
   ExecuteNow
   BufferInput
   Reject

# 打断关系实现示例

1. 实现原则
   打断关系分为两层：
   第一层是程序统一维护的动作阶段语义；
   第二层是策划可配置的阶段打断规则。
   程序不自动推断“哪一段是前摇、哪一段是后摇”，而是由动作自身在运行时显式上报当前阶段。
2. 前摇和后摇的定义方式
   以普通攻击为例：
   `Startup` 视为前摇，表示动作开始后到攻击真正生效前的阶段；
   `Active` 视为有效段，表示命中判定或攻击表现生效的阶段；
   `Recovery` 视为后摇，表示有效段结束后到动作逻辑结束前的阶段；
   `LogicEnded` 表示逻辑上已经允许接下一动作，但表现层动画可能仍在播放。
3. 策划如何定义阶段边界
   主要通过动画通知或 Montage Section 切换点定义，而不是让程序按固定时长自动切段。
   例如：
   普攻开始时进入 `Startup`
   攻击命中窗口开始时切到 `Active`
   命中窗口结束并进入收招时切到 `Recovery`
   允许衔接下一动作时切到 `LogicEnded`
4. 策划如何配置打断规则
   策划不需要配置复杂状态机，只需要配置“某阶段允许被哪些输入打断”。
   例如普通攻击默认规则可以是：
   `Startup` 不允许被 `Dodge` 打断
   `Active` 不允许被 `Dodge` 打断
   `Recovery` 允许被 `Dodge` 打断
   `Recovery` 允许被 `Skill` 打断
5. 程序实际判定流程
   当玩家输入一个新动作时，公共语义层读取当前动作上下文：
   当前动作类型是什么；
   当前动作阶段是什么；
   当前阶段是否允许被该输入类型打断。
   若允许，则立即中断当前动作并执行新动作；
   若不允许但该输入支持预输入，则进入预输入；
   若两者都不满足，则拒绝输入。
6. 普通攻击示例
   示例动作：
   普通攻击第 1 段 `GA_NormalAttack_01`
   示例时间线：
   0.00 秒进入 `Startup`
   0.18 秒进入 `Active`
   0.32 秒进入 `Recovery`
   0.48 秒进入 `LogicEnded`
   0.60 秒 Ability 完全结束
   示例规则：
   在 0.00 到 0.18 秒期间，闪避输入不能立即打断，只能拒绝或进入预输入；
   在 0.18 到 0.32 秒期间，闪避同样不能立即打断；
   在 0.32 到 0.48 秒期间，若配置允许，则闪避可以立即打断；
   在 0.48 秒后，即使动画还没完全播完，也允许后续动作直接衔接。
7. 实现建议
   正式资源以动画通知和动作逻辑事件作为阶段切换主依据；
   C++ 层可保留时间兜底配置，用于测试资源、通知缺失或纯逻辑动作场景；
   但正式项目不应长期依赖纯时长自动推断前摇和后摇。

# 公共标签建议

1. 限制类 Tag
   `State.ActionLocked`
   `State.CannotAttack`
   `State.CannotDodge`
   `State.CannotGuard`
   `State.CannotInput`
2. 动作态 Tag
   `State.Action.NormalAttack`
   `State.Action.Dodge`
   `State.Action.Guard`
   `State.Action.HitReaction`
3. 阶段类 Tag
   `State.Phase.Startup`
   `State.Phase.Active`
   `State.Phase.Recovery`
4. 能力类 Tag
   `Ability.Interrupt.Dodge`
   `Ability.Interrupt.Skill`
   `Ability.Interrupt.Guard`
5. 说明
   这里先给出命名方向，资深程序实现时可以按项目现有 Tag 规范调整，但必须保持“限制类、动作态、阶段类、能力类”四层结构分离。

# 蓝图与C++分工

1. 哪些部分必须由C++实现
   动作上下文结构、输入评估、打断规则、动作逻辑结束事件、统一状态写入与清理。
2. 哪些部分适合交给蓝图配置或表现层处理
   具体动画通知触发时机、不同动作的阶段切换点、镜头和特效表现、个别动作的特例配置。
3. 两者之间如何通信
   Ability 基类在 C++ 中暴露：
   动作开始上报接口
   阶段切换上报接口
   动作逻辑结束上报接口
   蓝图通过这些接口把动画时序和表现层时机同步给公共语义层。

# 风险点

1. 可能出现的性能问题
   若所有输入判断都依赖蓝图临时拼装，会有大量重复 Tag 查询和状态判断，因此输入评估和打断规则必须常驻在 C++。
2. 可能出现的联机同步问题
   当前阶段虽不做联机验收，但动作阶段和逻辑结束事件必须具备稳定可复制语义，否则后续很难补同步。
3. 可能出现的状态错乱问题
   若 Ability 自己结束时不通知统一组件，或者动画结束和逻辑结束混用，最容易导致卡在错误阶段、预输入不结算、错误允许打断。
4. 需要重点关注的边界条件
   同一帧收到“受击打断”和“动作逻辑结束”
   当前动作刚逻辑结束就收到新输入
   前摇特例可闪避打断
   格挡失败动作允许被闪避取消
   受击导致预输入必须清空

# 联调与验收

1. 实现完成后需要和哪些模块联调
   普通攻击 Ability、闪避 Ability、格挡 Ability、受击响应模块、预输入模块、角色输入桥接层、动画蓝图。
2. 最少需要验证哪些场景
   普攻前摇不能被闪避打断；
   普攻后摇能被闪避打断；
   动作逻辑结束先于动画播完时，预输入仍能正常执行；
   受击打断后当前动作状态被正确清理；
   格挡失败动作期间允许闪避取消；
   当前动作为闪避时，其他输入进入预输入而不立即执行。
3. 出现问题时优先检查哪些节点
   `BeginCombatAction` 是否被调用；
   `UpdateCombatActionPhase` 是否正确写入阶段；
   `ResolveInterruptRule` 是否按当前阶段生效；
   `NotifyCombatActionLogicEnd` 是否广播；
   `EndCombatAction` 是否统一清理上下文和阶段 Tag。

# 实施拆分与任务分派

1. 子任务A：定义公共枚举与数据结构
   目标：
   先把动作类型、动作阶段、输入评估结果、结束原因、动作上下文结构定死。
   交付结果：
   后续各模块可直接引用的头文件和基础结构。

2. 子任务B：实现 `UCombatActionComponent`
   目标：
   完成动作上下文的创建、更新、结束、查询。
   交付结果：
   可被角色挂载并被 Ability 调用的公共组件。

3. 子任务C：实现输入评估与打断规则
   目标：
   跑通立即执行、进入预输入、拒绝三类输入结果，并支持按阶段判断打断。
   交付结果：
   可给预输入模块和各战斗 Ability 共用的评估接口。

4. 子任务D：实现动作逻辑结束事件
   目标：
   把逻辑结束与表现结束拆开，打通给预输入和状态清理的统一通知。
   交付结果：
   统一结束事件和蓝图可监听桥接。

5. 子任务E：接入三个样板动作
   目标：
   先选普攻、闪避、格挡三个 Ability 做样板接入，验证公共语义层不是空架子。
   交付结果：
   三个动作能通过统一语义层切换阶段、判断打断、正确结束。

# 建议实施顺序

1. 先完成子任务A，先把命名和结构锁住。
2. 再完成子任务B和子任务C，形成真正可用的底座。
3. 然后完成子任务D，把“动作逻辑结束事件”打通。
4. 最后用子任务E做验证接入，再交给后续普攻、闪避、格挡模块继续展开。
