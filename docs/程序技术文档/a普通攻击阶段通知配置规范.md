# 文档用途

1. 本文档用于给策划、动画和程序联调时统一“普通攻击阶段通知”的配置方式。
2. 当前目标是让普攻阶段边界优先由动画通知驱动，而不是长期依赖时长推断。
3. 本文档对应当前程序实现：
   `a普通攻击阶段标记与基础打断技术文档.md`

# 适用范围

1. 当前至少适用于普通攻击 `1 / 2 / 3` 三段 Montage Section。
2. 后续若技能、格挡、其他动作也要接阶段语义，可参考同一命名思路，但不要擅自改普通攻击既有通知名。

# 当前标准通知名

1. `CombatPhase_Active`
   含义：
   当前普攻段从前摇进入有效动作窗口。
2. `CombatPhase_Recovery`
   含义：
   当前普攻段从有效动作窗口进入收招阶段。
3. `CombatPhase_LogicEnded`
   含义：
   当前普攻段在战斗规则层面已经结束。

# 当前程序口径

1. `Startup`
   不需要单独配置 Notify；
   当前由普攻 Ability 激活成功后自动进入。
2. `Active / Recovery / LogicEnded`
   当前由 Montage Notify 名称驱动写入；
   Notify 名称必须与本文档保持一致。
3. 如果当前动画尚未配置 Notify：
   程序会使用当前段 Section 时长做最小时序兜底；
   但这只是临时联调手段，不应作为长期正式配置方式。

# 每段普攻的建议配置方式

1. `Attack_1`
   在第 1 段真正开始生效的帧放置：
   `CombatPhase_Active`
   在第 1 段有效动作结束的帧放置：
   `CombatPhase_Recovery`
   在第 1 段逻辑允许衔接下一动作的帧放置：
   `CombatPhase_LogicEnded`
2. `Attack_2`
   同样放置：
   `CombatPhase_Active`
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`
3. `Attack_3`
   同样放置：
   `CombatPhase_Active`
   `CombatPhase_Recovery`
   `CombatPhase_LogicEnded`

# 放置原则

1. `CombatPhase_Active`
   应放在这一段动作真正开始命中/生效的起点附近；
   不要放得过早，否则前摇会被错误视为有效阶段。
2. `CombatPhase_Recovery`
   应放在有效段结束后、收招开始的位置；
   当前默认“允许被 Dodge 打断”的最小阶段就是从这里开始。
3. `CombatPhase_LogicEnded`
   应放在战斗规则上允许当前动作结束、允许衔接后续动作的位置；
   它可以早于 Montage 完全播完。
4. `CombatPhase_LogicEnded`
   不要简单等同于动画最后一帧；
   否则它就失去了从“Montage 播放结束”中拆出逻辑结束语义的意义。

# 当前最小打断规则对应关系

1. `Startup`
   默认不允许被 Dodge 打断。
2. `Active`
   默认不允许被 Dodge 打断。
3. `Recovery`
   默认允许被 Dodge 打断。
4. `LogicEnded`
   视为当前普攻逻辑已结束，可允许后续动作衔接。

# 当前联调建议

1. 配完 Notify 后，进入游戏查看调试面板或结构化日志。
2. 当前应能看到阶段顺序：
   `Startup -> Active -> Recovery -> LogicEnded`
3. 如果阶段顺序不对，优先检查：
   Notify 名称是否完全一致；
   Notify 是否放在了错误段落；
   Section 是否切到了错误动画位置。
4. 如果日志里一直看不到 `EnterPhase Active / Recovery / LogicEnded`：
   优先检查 Montage 是否真实触发了对应 Notify。

# 常见错误

1. 只配置了 `Active`，没配置 `Recovery / LogicEnded`。
2. `LogicEnded` 被放到动画最后一帧，导致它和 Montage End 没有区别。
3. 第 2 段或第 3 段沿用了错误的 Notify 位置，视觉表现与阶段日志对不上。
4. Notify 名称大小写或拼写不一致，导致程序没有识别到。

# 当前结论

1. 以后普通攻击相关动画，统一按这套 Notify 名称配置。
2. 程序侧已经支持这套命名规范；
   配置完成后，阶段语义、调试日志与最小 Dodge 打断规则都会按同一套边界工作。
