# 公共战斗语义层开工评估与实施拆分 - 主程序结论

## 结论摘要

1. 当前允许进入“公共战斗语义层”阶段，但前提是：
   `05-21-dodge-resource-local-acceptance` 已按其白盒清单完成 Unreal Editor 内本地验收，并被视为“基础闪避阶段已收口”。
2. 如果上述前提实际上尚未满足，则本 task 结论应立即回退为：
   暂不进入公共战斗语义层，先回到 `05-21-dodge-resource-local-acceptance` 完成资源、Notify、Root Motion 与 PIE 白盒验收。
3. 当前最适合开工的不是“一次性把整层公共语义全部做完”，而是先落第一轮最小正式底座：
   统一动作上下文；
   统一阶段语义与逻辑结束出口；
   统一普通攻击与基础闪避的打断判断接入点。

## 为什么现在可以进入

1. 普攻已经稳定落在 `UE5 + GAS + C++` 正式承载上，不再依赖旧 Character 普攻状态机。
2. 普攻已经具备明确阶段语义：
   `Startup / Active / Recovery / LogicEnded`。
3. 基础闪避已经具备正式 Ability 生命周期，而不是只剩“验证型 Dodge 打断”：
   有方向解析；
   有方向资源选择；
   有 `Root Motion` 要求；
   有冷却；
   有无敌帧 Tag；
   有输入失败与运行态调试口径。
4. 普攻与闪避之间已经出现足够稳定的共享概念，值得正式收束为公共层：
   动作类型；
   动作阶段；
   打断时机；
   输入限制；
   动作逻辑结束；
   动作态 Tag。

## 当前已稳定的共享概念

1. 动作 Ability 身份已经稳定：
   `Ability.NormalAttack`
   `Ability.NormalAttack.Segment1 / 2 / 3`
   `Ability.Dodge`
2. 动作态 Tag 已有正式出口：
   `State.Action.NormalAttack`
   `State.Action.Dodge`
   `State.CannotAttack`
   `State.CannotDodge`
   `State.CannotInput`
3. 普攻阶段语义已经形成真实代码口径，而不是只在文档中存在：
   `UTwoHeartsGA_NormalAttackBase::EnterCombatPhase`
   `NotifyCombatPhaseByName`
   `CanBeInterruptedByDodge`
   `TryInterruptByDodge`
4. Dodge 生命周期也已经具备可复用的正式概念：
   冷却 `Cooldown.Dodge`
   无敌帧 `State.Dodge.Invulnerable`
   动作完成 `Dodge_Finished`
   失败原因与调试事件输出。
5. 当前 Character 侧已经形成统一输入转发与调试承载入口，适合作为后续公共输入评估的上游接入口。

## 当前仍是过渡实现的部分

1. “动作阶段”目前只在普通攻击内正式维护，Dodge 还没有接入统一阶段上下文。
2. “打断规则”目前仍是普通攻击与 Dodge 之间的点对点硬编码，不是统一规则表或统一判定入口。
3. “逻辑结束”目前只在普通攻击内有清晰语义，尚未升级为全动作可订阅的公共事件。
4. “输入评估”已经正式收束到公共 Combat 层入口，并统一产出 `ExecuteNow / BufferInput / Reject` 三段结果；普攻与 Dodge 现已接入同一套评估口径。
5. 当前这一轮已留下“最小预输入”正式接入口，但还没有完成 Unreal Editor 内实际 PIE / 白盒输入联调，因此运行态手感、日志表现与边界时机仍需下一轮编辑器验证后再最终收口。

## 首轮实施边界

1. 当前目标：
   让普通攻击与基础闪避共享同一套最小动作上下文、阶段语义和逻辑结束出口。
2. 当前明确不做：
   不在这一轮直接接完整预输入；
   不把格挡、受击、伤害一起并入；
   不提前做面向所有未来系统的过重泛化；
   不把状态机回退到 `AtwoheartsCharacter`。
3. 当前阶段验收口径：
   普攻和 Dodge 能通过统一动作上下文暴露当前动作类型与阶段；
   Dodge 不再直接读取“某个普攻实现细节”才能判断打断；
   `LogicEnded` 成为可被公共层消费的标准出口；
   下一轮最小预输入已经有明确且稳定的接入口。

## 建议实施顺序

1. 先做“动作上下文最小底座”：
   建立共享 `ActionType / ActionPhase / ActionEndReason` 与当前动作上下文结构；
   提供开始动作、切阶段、逻辑结束、完全结束的统一出口。
2. 再做“普通攻击接入公共动作上下文”：
   让现有 `Startup / Active / Recovery / LogicEnded` 不再只留在普攻内部；
   把当前普攻调试态与公共动作上下文对齐。
3. 再做“基础闪避接入公共动作上下文与统一打断入口”：
   让 Dodge 激活、无敌帧、完成、冷却能通过公共层暴露关键状态；
   让“是否允许被 Dodge 打断”升级为公共判定接口，而不是只留在 `NormalAttackBase`。
4. 最后再做“输入评估与最小预输入预留接口”：
   这一轮只统一输入评估返回口径；
   不在本轮真正实现完整缓存执行。

## 四个 task 的固定顺序

1. 第一个 task：`combat-action-context-foundation`
   这是后面三个 task 的共同底座。
   如果这一层还没落，后面的普攻桥接、Dodge 桥接、输入评估都会各自偷带一份“临时上下文定义”，最后又回到分裂状态。
2. 第二个 task：`normal-attack-semantic-bridge`
   普攻是当前阶段语义最完整、代码口径最稳定的动作。
   应先把它接到公共层，作为公共动作上下文的第一份真实样板。
3. 第三个 task：`dodge-semantic-bridge-and-interrupt-unification`
   只有在公共动作上下文已经存在、普攻也已先桥接完成后，才适合把 Dodge 接进来，并把“是否允许被 Dodge 打断”升级成统一入口。
   否则很容易把 Dodge 桥接写成反向牵制普攻实现的特例逻辑。
4. 第四个 task：`combat-input-evaluation-preinput-hook`
   这个 task 必须排最后。
   因为输入评估本质上依赖“当前动作是什么、当前处于哪个阶段、当前动作是否已逻辑结束、当前是否允许打断”这些前置真相源。
   如果前 3 个 task 没先完成，这一层就只能继续读散落状态，不可能形成稳定的预输入接入口。

## 顺序不可打乱的原因

1. `combat-action-context-foundation` 是结构前提，不先做就没有统一真相源。
2. `normal-attack-semantic-bridge` 是首个样板，不先做就无法验证公共层是否真的能承载现有正式动作。
3. `dodge-semantic-bridge-and-interrupt-unification` 依赖前两项，否则打断规则仍会停留在点对点硬编码。
4. `combat-input-evaluation-preinput-hook` 依赖前三项，否则只能继续写成分散判断，无法作为“最小预输入”的稳定上游。

## 给资深程序的派工顺序口径

1. 先领 `combat-action-context-foundation`，完成后再允许创建或启动 `normal-attack-semantic-bridge`。
2. `normal-attack-semantic-bridge` 验收通过后，再进入 `dodge-semantic-bridge-and-interrupt-unification`。
3. 只有当前两项桥接都完成、公共动作上下文已经承载真实动作后，才允许进入 `combat-input-evaluation-preinput-hook`。
4. 若任一上游 task 验收失败，下游 task 不开工，直接回到上游修正，不并行硬推。

## 建议子 task 粒度

1. `combat-action-context-foundation`
   目标：
   落共享动作上下文、动作阶段枚举、动作结束原因和公共调试出口。
   单轮验收：
   普攻与 Dodge 都能读取同一份当前动作上下文定义，但不要求本轮完整迁完所有逻辑。
2. `normal-attack-semantic-bridge`
   目标：
   把普通攻击现有阶段流转、逻辑结束和调试态正式桥接到公共动作上下文。
   单轮验收：
   普攻 `Startup / Active / Recovery / LogicEnded` 能从公共层观察到，且不回归原有连段行为。
3. `dodge-semantic-bridge-and-interrupt-unification`
   目标：
   让 Dodge 接入公共动作上下文，并把“普通攻击可否被 Dodge 打断”的判断升级为公共入口。
   单轮验收：
   现有 Dodge 打断链路仍工作；
   普攻不再只靠私有实现细节暴露打断口径；
   公共层能读到 Dodge 的开始、逻辑结束和完成。
4. `combat-input-evaluation-preinput-hook`
   目标：
   统一输入评估结果与后续最小预输入接入口，但不在本轮真正完成完整预输入。
   单轮验收：
   至少能返回 Execute / Buffer / Reject 三种标准结果，并与现有普攻、Dodge 输入路径对齐。

## 回退条件

1. 若在接入公共层时发现 Dodge 资源、Notify 或白盒联调实际上并未稳定，则暂停公共层实现，回退到 `05-21-dodge-resource-local-acceptance` 补齐验收。
2. 若发现普通攻击当前阶段语义与真正联调表现仍不一致，则优先修正普通攻击阶段真相源，不要带着漂移语义直接抽公共层。
3. 若首轮实施开始后发现任务粒度仍过大，应继续按父子 task 规则下拆，而不是在一个 task 里把动作上下文、打断、预输入一起揉完。

## 给资深程序的直接开工口径

1. 这不是“重新设计一套战斗系统”，而是把已经在普攻与基础闪避里反复出现的稳定概念正式收口。
2. 第一轮优先保真，不优先做重抽象：
   先让现有真实逻辑接上公共层，再讨论更泛化的可配置规则。
3. 当前主程序建议以“动作上下文底座 -> 普攻桥接 -> Dodge 桥接 -> 输入评估预留”作为固定顺序，不反过来做。
