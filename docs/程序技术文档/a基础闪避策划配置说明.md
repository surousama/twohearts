# 文档用途

1. 本文档用于给策划和蓝图同学说明：当前“基础闪避正式落地”阶段，需要在角色资源和蓝图侧配置哪些内容。
2. 本文档只覆盖“当前基础闪避 Ability 已经完成 C++ 主体实现”后的配置工作，不展开后续公共语义层、预输入、受击命中判定和联机内容。
3. 当前目标是让策划能够按本文档完成最小可验收配置，并在场景内验证闪避是否已经正式成立。

# 当前结论

1. `UTwoHeartsGA_Dodge` 的 C++ 主体已完成。
2. 当前闪避已经具备：
   正式输入链路；
   普攻阶段打断衔接；
   方向解析；
   Root Motion 动作承载；
   冷却状态；
   无敌帧状态出口；
   调试日志与 HUD 字段。
3. 当前如果角色还“看起来不像正式闪避”，大概率不是 C++ 没接通，而是角色蓝图和资源配置还没补齐。

# 你现在需要配置的内容

1. 给角色配置 `8` 向闪避 `Montage`。
2. 给角色配置基础闪避参数：
   闪避冷却；
   无敌帧兜底时间；
   动画通知。
3. 检查 `Dodge` 输入是否真的映射到当前测试按键。
4. 进场景按本文档验收口径逐项验证。

# 当前配置入口

1. 当前配置入口在角色蓝图实例上，来源是 `AtwoheartsCharacter` 新增的 `DodgeConfig`。
2. 当前建议直接在玩家角色蓝图上配置，例如：
   `BP_ThirdPersonCharacter`
3. 当前程序字段名为：
   `DodgeConfig.DodgeMontageFallback`
   `DodgeConfig.DodgeMontageForward`
   `DodgeConfig.DodgeMontageForwardRight`
   `DodgeConfig.DodgeMontageRight`
   `DodgeConfig.DodgeMontageBackwardRight`
   `DodgeConfig.DodgeMontageBackward`
   `DodgeConfig.DodgeMontageBackwardLeft`
   `DodgeConfig.DodgeMontageLeft`
   `DodgeConfig.DodgeMontageForwardLeft`
   `DodgeConfig.DodgeCooldownSeconds`
   `DodgeConfig.DodgeInvulnerableStartSeconds`
   `DodgeConfig.DodgeInvulnerableDurationSeconds`

# 每个字段怎么配

## 1. 8 向 Dodge Montage

1. 含义：
   按当前闪避方向播放对应的闪避蒙太奇资源。
2. 当前要求：
   当前程序已支持 `8` 向独立资源配置。
3. 当前推荐配置：
   `Forward`
   `ForwardRight`
   `Right`
   `BackwardRight`
   `Backward`
   `BackwardLeft`
   `Left`
   `ForwardLeft`
4. 当前回退位：
   `DodgeMontageFallback`
   作用：
   当某一方向槽位没填时，回退到这套 Montage，避免直接放不出来。
5. 如果方向槽位和回退位都不配：
   当前闪避会被程序直接拒绝，不会进入正式闪避。
6. 当前正式要求：
   所有正式使用的 Dodge Montage 都应启用 `Root Motion`。
7. 当前建议 Notify 名称：
   `Dodge_InvulnerableBegin`
   `Dodge_InvulnerableEnd`
   `Dodge_Finished`

## 2. DodgeCooldownSeconds

1. 含义：
   两次闪避之间的最小间隔。
2. 当前建议起配：
   `0.8`
3. 当前程序表现：
   冷却期间再次按闪避，不会重复进入闪避。
4. 当前调试观察：
   HUD 上的 `dodge_cooldown_ready` 会反映当前是否恢复可释放。

## 3. DodgeInvulnerableStartSeconds

1. 含义：
   当对应 Montage 还没补 `Dodge_InvulnerableBegin` Notify 时，用于兜底进入无敌帧的时间。
2. 当前建议起配：
   `0.0`
3. 当前建议：
   第一轮联调优先从 `0.0` 开始；
   先确认基础逻辑稳定，再微调无敌帧起点。

## 4. DodgeInvulnerableDurationSeconds

1. 含义：
   当对应 Montage 还没补 `Dodge_InvulnerableEnd` Notify 时，用于兜底结束无敌帧的持续时间。
2. 当前建议起配：
   `0.22`
3. 当前调试观察：
   HUD 上的 `dodge_invulnerable` 会在无敌帧开始与结束时切换。
4. 当前注意：
   这一轮只要求“无敌帧状态出口正确”；
   还不要求完整验证“哪些攻击可被闪掉”的命中系统。

# 输入检查

1. 当前 `Dodge` 输入资产入口已经存在。
2. 如果你按键没有反应，先检查：
   `Input Mapping Context` 里是否已经把 `IA_Dodge` 绑定到目标按键；
   同一个按键是否还被 `Jump` 等其他输入占用。
3. 当前常见误判：
   如果同一按键同时给了 `Jump` 和 `Dodge`，容易把输入冲突误判成闪避逻辑没生效。

# 当前方向规则

1. 有移动输入时：
   闪避方向跟随当前移动输入。
2. 没有移动输入时：
   闪避方向默认沿角色当前面朝方向。
3. 当前程序已经支持基础 `8` 方向命名输出，并会按方向选择对应 Montage：
   `Forward`
   `ForwardRight`
   `Right`
   `BackwardRight`
   `Backward`
   `BackwardLeft`
   `Left`
   `ForwardLeft`
4. 当前如果某一方向 Montage 没配，会回退到 `DodgeMontageFallback`。
5. 当前如果方向 Montage 已配但没有启用 `Root Motion`，程序会拒绝本次正式闪避并输出黄色警告日志。

# 当前打断规则

1. 普攻 `Recovery / LogicEnded` 期间按闪避：
   允许打断并进入闪避。
2. 普攻 `Startup / Active` 期间按闪避：
   当前默认不允许直接打断。
3. 当前阶段如果闪避没放出来，不要第一反应就怀疑输入丢了；
   也可能是当前普攻阶段本来就不允许被闪避打断。

# 当前你在游戏里应该看到什么

1. 待机时按闪避：
   角色会进入闪避动作，并由 Root Motion 带出位移。
2. 有方向输入时按闪避：
   角色会朝输入方向闪。
3. 没方向输入时按闪避：
   角色会朝当前朝向闪。
4. 连按闪避：
   第二次会被冷却挡住，不应无间隔连续触发。
5. 普攻后摇按闪避：
   普攻会被中断，角色进入闪避。
6. 普攻前摇或生效段按闪避：
   当前默认不会立刻进入闪避。

# 当前调试怎么看

1. HUD 里重点看这几个字段：
   `dodging`
   `direction`
   `invulnerable`
   `cooldown_ready`
2. 当前关键日志事件：
   `DodgeActivate`
   `DodgeRejected`
   `DodgeDirectionResolved`
   `DodgeInterruptedNormalAttack`
   `DodgeInvulnerableBegin`
   `DodgeInvulnerableEnd`
   `DodgeFinished`
   `DodgeCooldownReady`
   `DodgeMontageSelected`
3. 当前如果看到：
   `dodging=YES`
   `direction` 正常变化
   `invulnerable` 会开关
   `cooldown_ready` 会从 `NO` 再回到 `YES`
   但角色动作不对
   优先排查当前方向 Montage 是否启用 Root Motion，以及 Notify 是否已补齐。

# 建议的第一轮默认配置

1. `DodgeMontageFallback`
   建议先配一套通用可用的基础闪避 Montage，作为兜底。
2. `DodgeMontageForward / ForwardRight / Right / BackwardRight / Backward / BackwardLeft / Left / ForwardLeft`
   按你现有 8 向资源分别填入。
3. `DodgeCooldownSeconds`
   `0.8`
4. `DodgeInvulnerableStartSeconds`
   `0.0`
5. `DodgeInvulnerableDurationSeconds`
   `0.22`
6. 每个方向 Montage 补齐：
   `Dodge_InvulnerableBegin`
   `Dodge_InvulnerableEnd`
   `Dodge_Finished`

# 第一轮验收清单

1. 待机按闪避，能稳定出动作。
2. 前后左右和斜方向输入下，闪避方向符合预期。
3. 不输入方向时，闪避沿当前朝向发生。
4. 普攻后摇按闪避，能正确打断普攻。
5. 普攻前摇和生效段按闪避，当前不会直接打断。
6. 闪避期间，不能被普通攻击输入立刻打断。
7. 闪避结束后，可以立刻接后续动作。
8. 冷却期间连续按闪避，不会重复触发。
9. `dodge_invulnerable` 会正常进入和退出。

# 当前常见问题

1. 问题：
   按了闪避直接被拒绝，黄色日志提示 Root Motion 问题。
   先查：
   当前方向对应的 Montage 是否已启用 `Root Motion`。
2. 问题：
   闪避方向不对。
   先查：
   是否在移动输入下测试；
   当前测试时角色朝向和输入方向是否被误读；
   是否误以为锁定状态要单独一套方向规则。
3. 问题：
   闪避按了没出。
   先查：
   输入映射；
   是否正处于普攻不可打断阶段；
   是否被冷却或限制 Tag 挡住。
4. 问题：
   闪避看起来太长或太短。
   先查：
   当前方向 Montage 的实际时长、Root Motion 节奏和 `Dodge_Finished` Notify 落点是否合理。

# 当前边界

1. 本文档不负责定义完整命中规避规则。
2. 本文档不负责定义正式闪避成功收益。
3. 本文档不负责定义完整预输入行为。
4. 本文档不负责联机同步配置。
