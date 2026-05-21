# 文档用途

1. 这份文档用于给资深程序直接开工，完成《双心印》第二章当前阶段“基础闪避正式落地”的实现。
2. 本文档是当前新的直接实施文档，目标是把现有“仅用于验证普攻阶段打断的最小 Dodge Ability”，补成可正式验收的基础闪避能力。
3. 本文档默认以上游主文档 [a第二章基础战斗模块开发总文档](./a第二章基础战斗模块开发总文档.md) 为前提。
4. 当前若需要给策划或蓝图同学落资源与参数配置，配套说明见：
   [a基础闪避策划配置说明](./a基础闪避策划配置说明.md)

# 基础信息

1. 功能名称：基础闪避正式落地
2. 上游设计文档：[a双心印战斗系统框架](../a双心印战斗系统框架.md) 第 `2.6` 节
3. 上游主文档：[a第二章基础战斗模块开发总文档](./a第二章基础战斗模块开发总文档.md)
4. 当前验收口径：单角色本地验证
5. 当前阶段目标：让闪避从“验证型 Dodge Ability”进入“正式基础动作 Ability”

# 当前主程序结论

1. 当前没有阻断本阶段开工的严重漏洞。
2. 当前普通攻击已经具备正式 Ability 承载和基础阶段语义，可以作为闪避接入的打断对象。
3. 当前 `UTwoHeartsGA_Dodge` 已经存在，但它只够做“普攻后摇可否被 Dodge 打断”的验证，不够承担正式闪避动作。
4. 本阶段正确目标不是直接抽公共语义层，也不是先做完整预输入，而是先把闪避自身的正式能力立住。
5. 只有当普攻和闪避都形成稳定 Ability 承载后，后续抽公共语义层才更稳。

# 本阶段完成标准

1. 角色具备正式可触发的基础闪避 Ability，而不只是打断型测试能力。
2. 闪避输入走 `AbilitySystem` 正式链路，不回退 Character 临时状态机。
3. 闪避方向按当前移动输入决定；若无移动输入，则默认沿角色当前面朝方向闪避。
4. 当前阶段至少支持基础 `8` 方向判定口径，但不要求本轮一次做完复杂差异化表现。
5. 闪避动作具备正式动画/位移承载，不允许只有日志和立即结束。
6. 闪避具备基础冷却。
7. 闪避具备基础限制条件判定，例如被禁止闪避时不可释放。
8. 闪避具备基础无敌帧状态表达，哪怕本轮先只完成最小状态出口，也不能完全没有接口。
9. 闪避与当前普攻阶段语义打通：
   普攻 `Recovery` 可被闪避打断；
   普攻 `Startup / Active` 默认不可被闪避直接打断。
10. 闪避结束后，角色应允许继续衔接后续动作。

# 本阶段明确不做

1. 不在本阶段实现正式通用预输入。
2. 不在本阶段实现完整公共战斗语义层。
3. 不在本阶段实现完整受击、伤害、格挡联动。
4. 不在本阶段完成“闪避成功收益”的正式资源回复结算。
5. 不在本阶段实现完整“哪些攻击可被闪避规避”的命中判定系统。
6. 不在本阶段做联机同步改造。
7. 不在本阶段为了闪避动作去重做新的角色战斗基类。

# 当前代码现状

1. 角色已存在 Dodge 输入入口：
   `AtwoheartsCharacter::Dodge`
2. 角色已把 Dodge 输入转发到：
   `HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Dodge)`
3. 默认战斗 Ability 授予链路中，已包含：
   `UTwoHeartsGA_Dodge`
4. 当前 `UTwoHeartsGA_Dodge` 已具备：
   `Ability.Dodge` 资产 Tag；
   `State.Action.Dodge` 激活态 Tag；
   查询当前激活普攻 Ability；
   调用 `TryInterruptByDodge` 执行最小打断判断。
5. 当前 `UTwoHeartsGA_Dodge` 尚不具备：
   正式闪避方向判定；
   正式闪避 Montage 或位移执行；
   基础冷却；
   基础无敌帧状态表达；
   正式闪避调试状态；
   闪避动作生命周期日志与联调观察口径。
6. 当前已验证：
   `Dodge` 输入可以通过 `HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Dodge)` 进入 GAS；
   最小 Dodge 可以查询当前普攻阶段并正确拒绝 `Startup / Active` 阶段的打断请求；
   当前若缺少正式闪避资源，可能只看到普攻被允许或拒绝打断，而看不到完整闪避表现。
7. 当前最新正式口径：
   基础闪避正式版按 `Root Motion` 承载；
   闪避位移不再以 `SetActorLocation` 硬推为正式主路径；
   无敌帧开始、无敌帧结束和动作逻辑结束，优先由 Montage Notify 驱动；
   配置时间窗仅作为未补齐 Notify 时的最小兜底。

# 当前阶段正式承载结论

1. 本阶段闪避正式承载位置，应落在 `Gameplay Ability`。
2. `AtwoheartsCharacter` 本阶段继续只负责：
   输入绑定；
   输入转发；
   角色级资源配置入口；
   调试承载。
3. 闪避方向判定、闪避执行、冷却判定、无敌帧状态写入、动作结束收尾，应由 Dodge Ability 承载。
4. 本阶段允许“角色持有闪避配置，Ability 持有闪避逻辑”的最小分工。
5. 本阶段不建议为闪避单独提前抽公共动作组件；统一收束留到公共语义层阶段。
6. 在当前正式口径下，`Root Motion` 驱动的位移表现属于闪避动作本体的一部分，应由方向 Montage 资源承载。

# 建议实现结构

1. 保留并扩展正式闪避 Ability：
   `UTwoHeartsGA_Dodge`
2. 如有必要，新增闪避配置结构，但应保持轻量：
   方向判定参数；
   闪避持续时间；
   闪避位移参数；
   无敌帧起止时间；
   调试字段。
3. 当前推荐代码落点：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
4. 角色侧配置入口建议落点：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
5. 如需补充 Tag 出口：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`

# 建议职责拆分

1. `UTwoHeartsGA_Dodge`
   负责：
   闪避激活条件校验；
   读取角色输入与方向；
   决定本次闪避方向；
   发起方向对应的 Root Motion 闪避 Montage；
   写入闪避动作状态；
   写入和清理无敌帧状态；
   处理普攻打断；
   处理冷却与动作收尾；
   输出统一调试日志。
2. `AtwoheartsCharacter`
   负责：
   提供 Dodge 输入；
   提供角色当前移动输入或面朝方向读取入口；
   提供闪避资源配置入口；
   提供闪避调试状态承载。
3. 当前普攻 Ability
   继续负责：
   暴露 `CanBeInterruptedByDodge / TryInterruptByDodge`；
   不承担闪避动作执行本身。

# 关键规则口径

1. 输入规则
   闪避通过玩家主动按键触发，按下即触发；
   当前不支持长按和按下/松开派生逻辑。
2. 方向规则
   读取当前移动输入方向；
   若存在移动输入，则按输入方向闪避；
   若无移动输入，则沿角色当前面朝方向闪避；
   当前锁定与非锁定状态共用同一套方向判定。
3. 打断规则
   若当前有激活中的普攻 Ability，则先查询其当前阶段；
   `Recovery` 允许被闪避打断；
   `Startup / Active` 默认不允许被闪避直接打断；
   若当前普攻不可被打断，则本次闪避直接拒绝，不在本阶段私自补局部预输入。
4. 闪避动作规则
   闪避本身视为一个完整动作，本阶段不再额外区分前摇和后摇；
   闪避执行期间，不允许其他主动动作立即打断当前闪避；
   闪避结束后允许立刻衔接后续动作。
5. 冷却规则
   闪避具有独立冷却；
   当前冷却可由 `Gameplay Effect` 或阶段内最小方案实现，但正式入口必须留在 GAS。
6. 无敌帧规则
   当前采用无敌帧式闪避；
   本阶段至少要提供“当前角色是否处于闪避无敌帧中”的稳定状态出口；
   具体“哪些命中可被闪避规避”的完整判定，可以留到后续命中/受击阶段接入。

# 建议 Tag 口径

1. 当前已存在：
   `Ability.Dodge`
   `State.Action.Dodge`
2. 本阶段建议补充：
   `State.CannotDodge`
   `State.Dodge.Invulnerable`
   `Cooldown.Dodge`
3. 说明
   当前阶段先把限制、动作态、无敌帧态和冷却出口立住；
   更完整的公共 Tag 分类，留待公共语义层阶段统一收束。

# 关键函数建议

1. 函数名：`ActivateAbility`
   作用：
   闪避激活总入口；
   校验角色、输入、限制条件、冷却状态；
   如当前普攻可被打断，则先中断普攻，再进入闪避执行。
2. 函数名：`ResolveDodgeDirection`
   作用：
   读取角色当前输入方向；
   若无输入则回退到面朝方向；
   输出本次闪避使用的世界方向或局部方向。
3. 函数名：`StartDodgeExecution`
   作用：
   启动方向对应的 Root Motion 闪避 Montage 和调试状态；
   写入 `State.Action.Dodge`。
4. 函数名：`BeginInvulnerabilityWindow`
   作用：
   写入 `State.Dodge.Invulnerable`；
   记录无敌帧开启。
5. 函数名：`EndInvulnerabilityWindow`
   作用：
   清理 `State.Dodge.Invulnerable`；
   记录无敌帧结束。
6. 函数名：`FinishDodge`
   作用：
   清理闪避运行态；
   结束 Montage/位移承载；
   结束 Ability；
   保证后续动作可正常衔接。

# 逻辑链条

1. 玩家按下 Dodge 输入。
2. `AtwoheartsCharacter::Dodge` 把输入送进 `HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Dodge)`。
3. `UTwoHeartsGA_Dodge::ActivateAbility` 被触发后，先做角色与限制条件校验。
4. 若当前有普攻在执行，则先走 `TryInterruptByDodge` 判定：
   可打断则中断普攻；
   不可打断则拒绝本次闪避。
5. 通过 `ResolveDodgeDirection` 计算本次闪避方向。
6. 通过 `StartDodgeExecution` 发起闪避表现与最小位移。
7. Montage Notify `Dodge_InvulnerableBegin` 或兜底时间窗触发 `BeginInvulnerabilityWindow`。
8. Montage Notify `Dodge_InvulnerableEnd` 或兜底时间窗触发 `EndInvulnerabilityWindow`。
9. Montage Notify `Dodge_Finished` 或 Montage 自然完成后执行 `FinishDodge`，结束当前 Ability。

# 蓝图与 C++ 分工

1. 必须由 C++ 实现的部分
   激活条件；
   打断判定；
   方向规则；
   无敌帧状态写入与清理；
   冷却与基础状态收尾；
   调试日志与联调输出。
2. 适合交给蓝图或资源配置的部分
   具体 Dodge Montage；
   方向差异化表现；
   动画通知时机；
   镜头、特效、位移曲线等表现层内容。
3. 通信方式
   C++ 暴露最小闪避执行与状态接口；
   蓝图通过 Montage/Notify/配置资源驱动表现；
   但最终状态口径仍以 C++ 为准。

# 当前建议的最小资源配置

1. Dodge 输入资产：
   当前已存在 `DodgeAction` 入口，可继续沿用。
   联调注意：
   若测试键位占用了 `Jump`，需要先在 `Input Mapping Context` 中移除 `IA_Jump` 的同键位映射，避免把输入冲突误判成 Dodge 逻辑异常。
2. Dodge 动画资源：
   当前正式口径为 `8` 向独立或半独立 Dodge Montage；
   正式资源应启用 Root Motion；
   若个别方向资源暂未补齐，可临时回退到 `Fallback Montage`，但不建议作为最终验收状态。
3. 方向与位移参数：
   方向由角色输入决定；
   正式位移距离以 Root Motion 资源为主，不再以代码距离硬推为正式主路径。
4. 无敌帧时机：
   当前正式口径优先通过 Notify 驱动；
   时间窗口仅保留为未补齐 Notify 时的兜底。
5. 当前建议 Notify 名称：
   `Dodge_InvulnerableBegin`
   `Dodge_InvulnerableEnd`
   `Dodge_Finished`

# 联调与验收口径

1. 角色在待机状态下按闪避，能够稳定触发闪避动作。
2. 有移动输入时，闪避方向与输入方向一致。
3. 无移动输入时，闪避方向默认为当前面朝方向。
4. 普攻 `Recovery` 期间按闪避，当前普攻会被正确打断并进入闪避。
5. 普攻 `Startup / Active` 期间按闪避，当前默认规则下不会直接打断普攻。
6. 闪避动作执行期间，不应被普通攻击等其他主动动作立即打断。
7. 闪避动作结束后，角色能够立刻恢复后续动作衔接能力。
8. 闪避冷却生效时，连续触发不会重复进入闪避。
9. 闪避无敌帧开始与结束时，调试状态能够正确反映。
10. 游戏内不应再只有“Dodge 打断普攻日志”，而没有正式闪避动作表现。

# 调试与排查建议

1. 建议新增或补充日志事件：
   `DodgeActivate`
   `DodgeRejected`
   `DodgeDirectionResolved`
   `DodgeInterruptedNormalAttack`
   `DodgeInvulnerableBegin`
   `DodgeInvulnerableEnd`
   `DodgeFinished`
2. 若当前测试场景已有屏幕调试覆盖或 `HUD`，建议补最小字段：
   `dodging`
   `dodge_direction`
   `dodge_invulnerable`
   `dodge_cooldown_ready`
3. 当前普攻调试面板已收敛为优先查看：
   `phase`
   `dodge`
   `logic_end`
   以及 `InterruptCheck / InterruptedByDodge / LogicEnded` 等关键事件。
4. 出现问题时优先检查：
   Dodge 输入是否进入 `HandleAbilityInputPressed`；
   `UTwoHeartsGA_Dodge` 是否被正确授予；
   是否被限制 Tag 或冷却挡住；
   当前普攻是否错误阻断了闪避；
   当前方向 Dodge Montage 是否已配置；
   当前方向 Montage 是否启用 Root Motion；
   `Dodge_InvulnerableBegin / Dodge_InvulnerableEnd / Dodge_Finished` 是否已正确配置。

# 风险点

1. 最大风险是把闪避做成“能打断普攻，但自己没有正式动作生命周期”的半成品。
2. 第二个风险是把方向、位移和无敌帧时机偷偷散到 Character 或蓝图里，导致后续公共语义层很难统一。
3. 第三个风险是为了赶进度，直接在闪避里自己发明一套临时输入缓存逻辑，后续会和预输入系统冲突。
4. 第四个风险是把闪避冷却做成完全脱离 GAS 的临时逻辑，后续再接正式冷却会返工。

# 完成后需要回写的内容

1. 在 [a第二章基础战斗模块开发总文档](./a第二章基础战斗模块开发总文档.md) 中回写：
   本次实际完成结论；
   代码落点；
   当前已知限制；
   下一步是否适合进入公共语义层。
2. 如本阶段产生稳定有效的专门配置规范，再决定是否补单独留档文档；
   否则优先回收进主文档，不新增无必要碎片文档。
