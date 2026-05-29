# 玩家受击表现正式化：技术实现备注

## 角色判定

当前执行角色：`资深程序`

职责边界：
- 按既有 `PRD` 与第二章当前阶段顺序执行实现
- 重点关注 `C++` 状态流、模块边界、`Blueprint` / `AnimBP` 分工与可联调性
- 不重写角色总状态机，不另起平行受击真相源

## 当前实现判断

1. 玩家侧命中 / 伤害 / Guard 改写 / 受击进入与自动恢复，真相源已经在 `UTwoHeartsHostileAttackReceiverComponent`
2. `HitReactionType` 与四向 `DirectionType` 已存在，且已接入公共动作上下文与动作打断
3. 当前缺口主要是“正式可见的受击表现层”，不是状态本身缺失
4. 本轮可接受“显式标注为过渡”的实现，但不能伪装成长期正式结构

## 本轮建议实现路线

采用“`C++` 真相源不变 + 角色配置桥接 + `Blueprint/AnimBP` 表现消费”的最小过渡方案：

1. `UTwoHeartsHostileAttackReceiverComponent`
   - 继续作为玩家受击状态真相源
   - 继续负责进入受击、恢复、Guard 改写撤销、动作打断
   - 原则上不新增第二套受击状态

2. `AtwoheartsCharacter`
   - 仅补“受击表现配置桥接”与只读查询入口
   - 负责把 `(HitReactionType, DirectionType)` 映射到具体 `Montage` / 表现配置
   - 不承接“是否进入受击、何时结束受击”的业务真相

3. `Blueprint / AnimBP`
   - 负责受击资源、Slot、混合规则、Montage 资产配置
   - 根据 `C++` 提供的配置入口或事件播放正式受击表现
   - 响应受击结束，避免状态已恢复但表现仍卡住

4. 恢复策略
   - 第一版保持与现有逻辑定时恢复一致
   - 若后续需要更精确的镜头/特效/输入恢复点，再考虑补 Montage Notify 精确控制

## 代码落点建议

1. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
   - 维持受击真相源
   - 如有必要，只补最小表现通知/配置消费对接点

2. `Source/twohearts/twoheartsCharacter.*`
   - 增加受击表现配置结构与查询接口
   - 作为角色级资源桥接层，不承接核心状态机

3. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
   - 如需验证 `Heavy` / `GuardBreak`，需要开放或补足最小测试配置，不再只固定 `Light`

## 策划 / 动画配置清单

### 必需配置

1. 配置位置：玩家角色蓝图（角色资源配置入口，通常是角色派生 `Blueprint`）
   - 需要资产/数据：
     - `Light_Front`
     - `Light_Back`
     - `Light_Left`
     - `Light_Right`
     - `Heavy_Front`
     - `Heavy_Back`
     - `Heavy_Left`
     - `Heavy_Right`
     - `GuardBreak_Front`
     - `GuardBreak_Back`
     - `GuardBreak_Left`
     - `GuardBreak_Right`
   - 用途：把 `HitReactionType + DirectionType` 正式映射到玩家受击表现资源
   - 备注：本轮只要求最小四向，不要求八向

2. 配置位置：玩家 `AnimBP`
   - 需要资产/数据：受击播放使用的 `Slot`、混合规则、必要时的全身/上半身覆盖策略
   - 用途：确保受击 `Montage` 能稳定播放并正确盖住当前动作
   - 备注：若本轮受击必须强打断，建议先走全身明确覆盖，后续再细化混合

3. 配置位置：受击 `Montage` 资源自身
   - 需要资产/数据：正确的段长、可选的 `Montage Notify / Branching Point`
   - 用途：支撑表现收尾、音效/镜头扩展与后续更精确的恢复时机控制
   - 备注：如果后续 `C++` 监听蒙太奇通知，必须使用真正的 `Montage Notify`，不要拿普通 `Anim Notify` 代替

### 联调建议配置

4. 配置位置：hostile probe 角色蓝图 / 攻击样本配置
   - 需要资产/数据：最小可切换的 `HitReactionType`（`Light / Heavy / GuardBreak`）、基础伤害、Guard 相关行为
   - 用途：白盒验证三类受击表现是否都能进入正确映射
   - 备注：当前 `Source` 样本更偏固定 `Light`；若要验 `Heavy / GuardBreak`，需补最小可调入口

## 验收关注点

1. 命中进入受击时，角色不再只剩日志 / HUD，而是有正式可见表现
2. `Light / Heavy / GuardBreak` 至少能走到不同配置
3. `Front / Back / Left / Right` 至少有稳定最小映射
4. `GuardBlocked` 或自动恢复后，动作上下文与受击表现同步收尾，不出现表现卡住
5. 本轮不扩散到八向库、死亡演出、霸体系统或完整 Guard 表现

## 当前已知限制

1. 第一版更偏“正式过渡方案”，仍以现有定时恢复为准
2. 若资源长度与逻辑时长差异过大，后续可能需要再补基于 `Montage Notify` 的精确恢复收口
3. 若后续确定长期正式承载要走受击专用 `Gameplay Ability`，应另开独立 task 收口，不在本轮强塞

## 本轮实现记录

1. `twoheartsCharacter.*`
   - 新增玩家受击配置结构：`Light / Heavy / GuardBreak` × `Front / Back / Left / Right / Fallback`
   - 新增受击 `Montage` 查询、播放、停止与恢复时长解析接口
   - 恢复时长支持取 `Montage` 时长或显式覆盖值，与原有最小时长取较大值

2. `TwoHeartsHostileAttackReceiverComponent.cpp`
   - 进入受击时正式消费角色侧受击配置并播放对应 `Montage`
   - `HitReactionState.Detail` 现在会带出“播了哪个 `Montage` / 为什么没播起来”的调试口径
   - 受击结束时会主动停止对应 `Montage`，避免状态已恢复但表现卡住

3. `TwoHeartsHostileAttackProbeCharacter.*`
   - 把 `HitReactionType`、`BaseDamage`、`bCanBeDodged` 开成可配置项
   - 便于白盒联调 `Light / Heavy / GuardBreak` 三类受击表现
