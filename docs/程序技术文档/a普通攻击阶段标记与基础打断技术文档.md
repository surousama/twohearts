# 文档用途

1. 这份文档用于给资深程序直接开工，完成《双心印》当前阶段“普通攻击阶段标记与基础打断”的实现。
2. 本文档是第二章当前新的直接实施文档，目标是在普通攻击已经正式迁入 `AbilitySystem` 的基础上，为其补齐可稳定扩展的动作阶段语义与最小打断规则。
3. 本文档默认以上一阶段 `a普通攻击完全切换到AbilitySystem技术文档.md` 已落地为前提。

# 基础信息

1. 功能名称：普通攻击阶段标记与基础打断
2. 上游设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 2 章
3. 上游总览文档：[a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md)
4. 上一阶段主文档：[a普通攻击完全切换到AbilitySystem技术文档](./a普通攻击完全切换到AbilitySystem技术文档.md)
5. 当前验收口径：单角色本地验证
6. 当前阶段目标：为正式 GAS 普攻补齐 `Startup / Active / Recovery / LogicEnded` 阶段语义，并支持最小基础闪避打断

# 当前主程序结论

1. 当前没有阻断本阶段开工的严重漏洞。
2. 普通攻击正式承载位置已经正确落到 Gameplay Ability，本阶段不需要回退承载结构。
3. 但当前实现仍主要依赖 `UAbilityTask_PlayMontageAndWait` 的完成/打断回调收束段落，还没有正式的动作阶段边界。
4. 如果现在直接叠加预输入、受击、闪避、格挡联动，会把后续规则继续压在 Montage 完成回调和零散布尔状态上，后续一定产生二次返工。
5. 因此本阶段正确顺序不是先做公共语义层，也不是先做预输入，而是先把普通攻击 Ability 自身的阶段语义立住，再在这个基础上接最小打断。
6. 本阶段完成后，下一步才适合继续做“最小预输入”。

# 本阶段完成标准

1. 普攻 `1 / 2 / 3` 三段 Ability 在运行时都具备明确的当前阶段状态。
2. 当前阶段至少包含：
   `Startup`
   `Active`
   `Recovery`
   `LogicEnded`
3. 阶段边界优先由动画通知或 Montage Notify 明确驱动，而不是继续主要依赖纯播放结束回调推断。
4. 普攻 Ability 内部能够统一响应阶段切换，并同步调试状态。
5. 新输入尝试打断当前普攻时，不再只看“Montage 是否播完”，而是能基于“当前阶段 + 新输入类型”做最小判定。
6. 本阶段至少支持一条基础规则：
   普攻在允许打断的阶段可被闪避输入打断；
   在不允许打断的阶段不可被闪避直接打断。
7. `LogicEnded` 语义与 Ability 完全结束语义区分明确，不混成同一个概念。

# 本阶段明确不做

1. 不在本阶段实现完整公共战斗语义层组件。
2. 不在本阶段把普攻、闪避、格挡、受击统一收口到一个通用动作组件。
3. 不在本阶段实现正式通用预输入系统。
4. 不在本阶段实现完整受击、伤害、格挡联动。
5. 不在本阶段做联机同步改造。
6. 不在本阶段为了阶段语义去重做新的角色战斗基类。
7. 不在本阶段把所有战斗阶段都抽成复杂数据驱动编辑器。

# 当前代码现状

1. 正式普攻已经由：
   `UTwoHeartsGA_NormalAttackBase`
   `UTwoHeartsGA_NormalAttack_1`
   `UTwoHeartsGA_NormalAttack_2`
   `UTwoHeartsGA_NormalAttack_3`
   承载。
2. 当前段落进入、Montage 播放、输入缓存、下一段推进和主要调试状态已经在 Ability 内。
3. 当前调试体系已从旧 Character 状态机切到 Ability 运行态快照。
4. 当前仍未正式建立：
   普攻阶段枚举；
   阶段切换通知入口；
   基于阶段的打断判断；
   `LogicEnded` 与 Ability 完全结束的明确分界。
5. 当前后续最容易失控的风险，是继续把“是否允许输入”“是否允许打断”“是否允许推进下一段”分散写进 Montage 回调和零散条件判断里。

# 当前阶段正式承载结论

1. 本阶段仍以 `UTwoHeartsGA_NormalAttackBase` 作为正式承载核心，不回退到 `AtwoheartsCharacter`。
2. Character 本阶段继续只保留：
   `ASC` 持有与初始化；
   输入转发；
   角色侧 Montage 与 Section 配置；
   调试状态读取与展示。
3. 普攻阶段状态、阶段切换、打断许可判断，必须由 Ability 侧正式承载。
4. 当前阶段允许“普攻 Ability 基类先内聚实现最小阶段语义”，不要求此刻就抽成公共组件。
5. 公共语义层仍是后置收束项，不是本阶段的承载目标。

# 建议实现结构

1. 新增动作阶段枚举
   建议名称：
   `ETwoHeartsCombatPhase`
   或保持项目命名风格的等价名称。
2. 阶段枚举至少包含：
   `None`
   `Startup`
   `Active`
   `Recovery`
   `LogicEnded`
3. 普攻 Ability 基类增加当前阶段运行态字段。
4. 普攻 Ability 基类增加统一阶段切换入口。
5. 普攻 Ability 基类增加“当前阶段是否允许被闪避打断”的最小查询能力。
6. 如当前动画资源允许，优先新增用于阶段切换的 Notify / NotifyState / Montage Notify。
7. 如本阶段需要最小蓝图桥接，允许在 Ability 基类暴露可被动画通知调用的阶段设置接口，但阶段写入源头仍应由 C++ 控制最终状态。

# 建议职责拆分

1. `UTwoHeartsGA_NormalAttackBase`
   负责：
   定义并维护当前普攻阶段；
   在激活时进入 `Startup`；
   响应动画通知切换到 `Active / Recovery / LogicEnded`；
   统一输出阶段日志和调试快照；
   统一处理“当前阶段是否允许闪避打断”；
   统一处理被打断后的收尾。
2. `UTwoHeartsGA_NormalAttack_1 / 2 / 3`
   继续只负责：
   段号定义；
   独立 Tag；
   下一段 Ability Tag；
   必要时允许覆盖个别阶段规则，但本阶段默认应共用同一套基类规则。
3. `AtwoheartsCharacter`
   继续不承担：
   普攻阶段判断；
   普攻打断判断；
   普攻状态机回退逻辑。

# 阶段语义定义

1. `Startup`
   含义：
   普攻段已开始，但尚未进入真正生效窗口。
   当前用途：
   用于表示前摇。
2. `Active`
   含义：
   普攻段处于有效动作窗口。
   当前用途：
   先作为有效段语义出口，为后续命中、伤害、受击联动预留位置。
3. `Recovery`
   含义：
   普攻有效段已结束，角色处于收招阶段。
   当前用途：
   这是本阶段最重要的最小打断窗口候选阶段。
4. `LogicEnded`
   含义：
   这段普攻在战斗规则上已经结束，可以允许后续动作衔接或彻底回待机。
   当前用途：
   把“逻辑结束”从“Montage 完全播完”中拆出来。

# 阶段切换实现建议

1. 当前段 Ability 激活成功后，默认立即进入 `Startup`。
2. `Startup -> Active`
   优先由动画通知触发。
3. `Active -> Recovery`
   优先由动画通知触发。
4. `Recovery -> LogicEnded`
   优先由动画通知触发。
5. Ability 最终结束时，不再把“完全结束”作为新的战斗阶段，只作为 Ability 生命周期收尾。
6. 如果当前测试资源暂时还未补齐所有通知，可允许保留最小临时兜底；
   但正式方向必须是“动画通知或动作逻辑事件驱动阶段切换”，不能继续长期依赖纯时长猜测。
7. 同一阶段重复写入时，应做最小幂等保护，避免调试状态抖动和重复事件污染日志。

# 打断规则的当前最小口径

1. 本阶段只实现“普攻被闪避打断”的最小规则，不扩展到技能、格挡、受击全套判定。
2. 判定输入建议采用：
   当前动作类型 + 当前阶段 + 新输入类型
3. 当前阶段建议默认规则：
   `Startup` 不允许被闪避打断；
   `Active` 不允许被闪避打断；
   `Recovery` 允许被闪避打断；
   `LogicEnded` 视为当前普攻逻辑已结束，可直接允许后续动作衔接。
4. 如果你们后续希望把“前摇可闪避取消”作为设计特例，也不要在本阶段直接写死成可取消；
   先按更保守的默认规则落地更稳。
5. 打断时的正确处理顺序应是：
   查询当前阶段；
   判定是否允许闪避打断；
   若允许，则结束当前普攻 Ability 并转入闪避 Ability；
   若不允许，则拒绝或按后续预输入阶段处理。
6. 本阶段不要求做成完整规则表系统，但接口命名和调用位置要为后续扩展预留稳定出口。

# 与输入缓存和下一段推进的关系

1. 本阶段不新增正式预输入系统。
2. 当前 Ability 内已有的“是否请求下一段”最小缓存逻辑可以保留。
3. 但当前应开始把“何时允许记录下一段请求”与“何时允许推进下一段”逐步挂到阶段语义上。
4. 当前建议口径：
   `Startup` 和 `Active` 允许记录最小下一段请求；
   段落真正推进仍由当前段收尾时机统一处理；
   `LogicEnded` 后不再继续接收本段新的连段请求。
5. 本阶段不用把它泛化成通用输入缓存，只要先避免后续继续写死在 Montage 结束条件上即可。

# Tag 与调试建议

1. 本阶段不强制一次补齐完整公共阶段 Tag 体系。
2. 但建议至少为后续扩展预留清晰命名方向，例如：
   `State.Phase.Startup`
   `State.Phase.Active`
   `State.Phase.Recovery`
   `State.Phase.LogicEnded`
3. 如果本阶段暂不真正写入这些公共 Tag，也应先把阶段枚举和调试输出稳定下来。
4. 当前调试面板建议新增或稳定展示：
   `phase`
   `interruptible_by_dodge`
   `logic_ended`
5. 当前关键日志建议至少包含：
   `EnterPhase`
   `LeavePhase`
   `InterruptCheck`
   `InterruptedByDodge`
   `LogicEnded`

# 推荐施工顺序

1. 先在普攻 Ability 基类中补齐阶段枚举、运行态字段和统一阶段切换入口。
2. 再补动画通知到 Ability 阶段写入的桥接。
3. 再让调试面板和日志稳定反映当前阶段。
4. 然后补“当前阶段是否允许被闪避打断”的最小查询接口。
5. 最后接入闪避输入打断当前普攻的最小链路。
6. 联调稳定后，再决定进入“最小预输入”阶段。

# 施工注意事项

1. 这次目标是“建立可信阶段边界”，不是“把所有战斗规则一次写完”。
2. 不要为了做阶段标记，反手新建一整套过早的公共组件框架。
3. 不要把阶段切换继续偷偷放回 `AtwoheartsCharacter`。
4. 不要把 `LogicEnded` 和 Montage 完成、Ability End 三者混成一件事。
5. 不要先用大量固定时长硬编码阶段边界，再把动画通知留到以后补；
   如果资源允许，通知边界本阶段就该立住。
6. 如果当前闪避 Ability 还未具备完整正式落地条件，本阶段也至少要把“普攻侧可被打断判定入口”先建好，避免后续闪避接入时再返工。

# 联调与验收口径

1. 点击一次普通攻击，第 1 段开始后应先进入 `Startup`。
2. 随动画通知推进，当前段应能明确切到 `Active`。
3. 当前段有效窗口结束后，应能明确切到 `Recovery`。
4. 当前段逻辑收尾点到来时，应能明确进入 `LogicEnded`。
5. 调试面板与日志中的阶段变化应与实际动画时序一致。
6. 在 `Startup` 期间触发闪避输入时，当前默认规则下不应直接打断普攻。
7. 在 `Active` 期间触发闪避输入时，当前默认规则下不应直接打断普攻。
8. 在 `Recovery` 期间触发闪避输入时，当前默认规则下应允许打断普攻。
9. 在 `LogicEnded` 之后，即使动画还有残余表现，也不应继续把本段普攻视为未结束。
10. 普攻 `1 / 2 / 3` 三段都应共用这套阶段语义，不允许只给第 1 段特殊硬编码。

# 风险点

1. 最大风险是阶段边界虽然名义上补了，但实际仍然主要靠 Montage 结束回调推断，导致后续预输入和打断规则没有真正脱离旧思路。
2. 第二个风险是动画通知接上了，但 Ability 运行态没有统一收口，最终又出现 Character、Ability、HUD 各看各的阶段状态。
3. 第三个风险是把 `LogicEnded` 和 Ability End 混用，后续很容易出现“逻辑已结束但输入仍被挡住”或者“动画未收尾但状态已被过早清空”的问题。
4. 第四个风险是为了快，直接把闪避打断逻辑写死在闪避入口里，而不是先查询普攻当前阶段，后续扩技能和格挡时会再次散掉。

# 完成后需要回写的文档

1. 在本文件内补充“实际代码文件、关键函数、最终实现口径、已知限制”。
2. 更新 [a第二章基础战斗模块技术总文档](./a第二章基础战斗模块技术总文档.md) 的当前入口和下一步顺序。
3. 如本阶段已明确完成，应同步更新 [a双心印开发进度简介](../a双心印开发进度简介.md) 中“当前下一步”描述。

# 建议代码落点

1. 普攻 Ability 基类：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
2. 普攻三段 Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.cpp`
3. 如需补公共枚举或阶段 Tag 出口：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`
   或项目现有合适的公共头文件位置。
4. 调试状态展示：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`

# 交接给资深测试的最小验证清单

1. 第 1 段普攻激活后，调试信息能显示 `Startup -> Active -> Recovery -> LogicEnded` 的顺序变化。
2. 第 2 段和第 3 段也能显示相同语义的阶段切换。
3. `Recovery` 期间触发闪避输入时，普攻能被正确打断。
4. `Startup` 和 `Active` 期间触发闪避输入时，普攻不会被错误打断。
5. `LogicEnded` 后，调试状态不会长时间卡在“仍视为攻击中”的错误状态。
6. 普攻三段切换时，阶段变化日志与当前 Section 表现一致。
7. 当前仍不应出现回落到旧 Character 普攻路径的情况。

# 进入下一阶段前的主程序判断标准

1. 只有当普攻阶段边界已经稳定，且最小基础打断已打通，才适合进入“最小预输入”。
2. 如果本阶段做完后仍然无法明确回答“当前动作现在处于 `Startup / Active / Recovery / LogicEnded` 的哪一段”，则说明这层底座还没立住，不适合继续往后叠功能。

# 当前已落地实现记录

1. 实现日期：
   2026-05-17
2. 本次实际完成结论：
   普攻 `1 / 2 / 3` 三段 Ability 已补齐 `Startup / Active / Recovery / LogicEnded` 阶段运行态；
   阶段切换已支持通过 Montage Notify 名称驱动，并保留最小时序兜底；
   当前最小 Dodge Ability 与输入入口已补齐，能够基于普攻当前阶段执行“是否允许打断”判定；
   普攻调试面板、结构化日志与屏幕覆盖调试信息已补充 `phase / interruptible_by_dodge / logic_ended` 输出。
3. 当前阶段验收口径仍为：
   单角色本地验证。

# 本次实际代码落点

1. 普攻阶段枚举：
   `twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`
2. 普攻 Ability 基类：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
3. 普攻三段 Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.cpp`
4. 最小 Dodge Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
5. Tag 扩展：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`
6. 角色输入与调试状态：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`
7. 动画配置规范文档：
   `docs/程序技术文档/a普通攻击阶段通知配置规范.md`

# 当前最终实现口径

1. `UTwoHeartsGA_NormalAttackBase`
   当前负责：
   维护当前普攻阶段；
   在 Ability 激活时进入 `Startup`；
   响应 Montage Notify 名称切换到 `Active / Recovery / LogicEnded`；
   在 Notify 尚未配置时，通过当前段 Section 时长按归一化时间做最小时序兜底；
   统一输出 `EnterPhase / LeavePhase / LogicEnded / InterruptCheck / InterruptedByDodge` 日志；
   统一处理“当前阶段是否允许被 Dodge 打断”的查询与执行。
2. `UTwoHeartsGA_NormalAttack_1 / 2 / 3`
   当前继续只负责：
   声明段号；
   声明独立 Ability Tag；
   声明下一段 Ability Tag。
3. `UTwoHeartsGA_Dodge`
   当前负责：
   作为本阶段最小 Dodge 输入承载；
   在激活时查找当前激活中的普攻 Ability；
   调用普攻 Ability 的“是否允许被 Dodge 打断”判定；
   若允许，则中断当前普攻；
   若不允许，则本次 Dodge 激活立即结束并输出日志。
4. `AtwoheartsCharacter`
   当前新增承担：
   Dodge 输入转发到 `HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Dodge)`；
   扩展普攻调试快照，展示 `phase / interruptible_by_dodge / logic_ended`。

# 关键函数与逻辑链条

1. 普攻阶段入口：
   `UTwoHeartsGA_NormalAttackBase::ActivateAbility`
   作用：
   初始化本段运行态；
   启动 Montage 播放；
   进入 `Startup`。
2. 阶段切换统一入口：
   `UTwoHeartsGA_NormalAttackBase::EnterCombatPhase`
   作用：
   统一做阶段切换幂等保护；
   记录 `LeavePhase / EnterPhase / LogicEnded` 日志；
   同步调试快照。
3. 动画通知桥接：
   `UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin`
   `UTwoHeartsGA_NormalAttackBase::NotifyCombatPhaseByName`
   作用：
   把 Notify 名称映射到 `Active / Recovery / LogicEnded` 阶段写入。
4. 临时时序兜底：
   `UTwoHeartsGA_NormalAttackBase::SchedulePhaseFallbacks`
   作用：
   在当前动画尚未配置 Notify 时，根据 Section 时长按归一化时间推进阶段；
   正式方向仍以 Notify 驱动为主。
5. 最小打断查询与执行：
   `UTwoHeartsGA_NormalAttackBase::CanBeInterruptedByDodge`
   `UTwoHeartsGA_NormalAttackBase::TryInterruptByDodge`
   作用：
   当前默认规则下，`Recovery / LogicEnded` 允许 Dodge 打断；
   `Startup / Active` 不允许。
6. Dodge 输入执行入口：
   `UTwoHeartsGA_Dodge::ActivateAbility`
   作用：
   激活后查找当前激活的普攻 Ability；
   若可打断，则中断普攻；
   若不可打断，则拒绝本次最小 Dodge 打断。
7. 调试状态同步：
   `AtwoheartsCharacter::SetNormalAttackDebugRuntimeState`
   作用：
   把 `segment / section / phase / interruptible_by_dodge / logic_ended` 写入角色调试快照；
   供 HUD、屏幕覆盖和结构化日志统一读取。

# 当前 Notify 配置口径

1. 当前标准 Notify 名称：
   `CombatPhase_Active`
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`
2. `Startup`
   不需要额外配置 Notify；
   当前由 Ability 激活成功后自动进入。
3. 当前动画 Notify 详细配置规范：
   见 [a普通攻击阶段通知配置规范](./a普通攻击阶段通知配置规范.md)

# 当前已知限制与后续注意事项

1. 当前阶段驱动虽然已支持 Notify，但由于现阶段资源尚未统一补齐，代码中保留了最小时序兜底；
   后续正式联调时应逐步把普攻段动画都补齐为 Notify 驱动。
2. 当前最小 Dodge Ability 只服务“普攻是否可被 Dodge 打断”的阶段验收，不代表正式 Dodge 表现层已完成。
3. 当前阶段仍未实现完整公共战斗语义层、通用预输入、受击、伤害、格挡联动和联机同步。
4. 当前仍不建议把更多战斗规则继续压回 Montage 完成回调；
   后续如扩展技能、格挡、受击，应复用当前阶段语义出口继续向前搭建。
