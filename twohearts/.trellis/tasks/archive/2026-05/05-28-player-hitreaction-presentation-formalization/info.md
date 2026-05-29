# 玩家受击表现正式化：设计配置与实现备注

## 角色判定

当前执行角色：`游戏设计师`

职责边界：
- 按既有 `PRD` 与第二章当前阶段顺序，把现有玩家受击逻辑落成可执行的资产配置清单
- 重点关注动画资产分组、`Montage` 组织、`Slot`/`Notify` 配置与联调口径
- 不直接改 `C++` 真相源，不重写角色总状态机，不另起平行受击状态

## 当前实现判断

1. 玩家侧命中 / 伤害 / Guard 改写 / 受击进入与自动恢复，真相源已经在 `UTwoHeartsHostileAttackReceiverComponent`
2. `HitReactionType` 与四向 `DirectionType` 已存在，且已接入公共动作上下文与动作打断
3. 当前缺口主要是“正式可见的受击表现层”，不是状态本身缺失
4. 本轮可接受“显式标注为过渡”的实现，但不能伪装成长期正式结构

## 当前资源判断

当前仓库里可直接用于本 task 的新资源主要在 `Content/HitReact/`，并且目前都是 `Animation Sequence`：

1. 轻受击候选：
   - `Hit_Combat_F_Seq`
   - `Hit_Combat_B_Seq`
   - `Hit_Combat_L_Seq`
   - `Hit_Combat_R_Seq`
2. 重受击候选：
   - `Hit_Large_Combat_F_Seq`
   - `Hit_Large_Combat_B_Seq`
   - `Hit_Large_Combat_L_Seq`
   - `Hit_Large_Combat_R_Seq`
3. `GuardBreak` 候选：
   - `Block_Hit_Break_Seq`
4. 当前不纳入本 task 主映射的资源：
   - `Block_Start_Seq` / `Block_Loop_Seq` / `Block_End_Seq`
   - `Block_Hit_Seq` / `Block_Hit_2_Seq`
   - `Parry_L_Seq` / `Parry_R_Seq`
   - `Hit_Death_Seq` / `Hit_Combat_Death_Seq` / `Hit_Large_Death_Seq` / `Hit_Large_Combat_Death_Seq`
   - 非 `Combat` 命名的 `Hit_F/B/L/R_Seq` 与 `Hit_Large_F/B/L/R_Seq`，默认只作为后备，不优先进入正式映射

## 设计师配置结论

### 哪些要做成 Montage

本轮建议先把下面 9 条 `Sequence` 升成正式 `AnimMontage`：

1. `Hit_Combat_F_Seq` → `AM_HitReact_Light_F`
2. `Hit_Combat_B_Seq` → `AM_HitReact_Light_B`
3. `Hit_Combat_L_Seq` → `AM_HitReact_Light_L`
4. `Hit_Combat_R_Seq` → `AM_HitReact_Light_R`
5. `Hit_Large_Combat_F_Seq` → `AM_HitReact_Heavy_F`
6. `Hit_Large_Combat_B_Seq` → `AM_HitReact_Heavy_B`
7. `Hit_Large_Combat_L_Seq` → `AM_HitReact_Heavy_L`
8. `Hit_Large_Combat_R_Seq` → `AM_HitReact_Heavy_R`
9. `Block_Hit_Break_Seq` → `AM_HitReact_GuardBreak`

说明：

1. `Light` 与 `Heavy` 已具备最小四向，应该优先正式化
2. `GuardBreak` 目前看起来只有 1 条资源，先做成 1 个独立 `Montage`，由配置层把它作为四向共用 fallback
3. 如果你后面补到了四向 `GuardBreak`，再扩成 `GuardBreak_F/B/L/R` 四条即可；本轮不阻塞

### 哪些先不要做成 Montage

1. `Block_Start_Seq` / `Block_Loop_Seq` / `Block_End_Seq`
   - 这些更像完整格挡表现链，不属于本 task 的“玩家受击表现正式化”边界
2. `Block_Hit_Seq` / `Block_Hit_2_Seq`
   - 更适合放进后续 Guard 表现 task，一起决定是普通格挡受击、招架反馈还是受击过渡
3. `Parry_L_Seq` / `Parry_R_Seq`
   - 属于招架/反制反馈，不要混入玩家被打受击
4. `Death` 相关
   - 明确超出本 task 范围
5. 非 `Combat` 命名的普通 `Hit_*`
   - 本轮只作为缺资源时的兜底参考，不建议先并入正式表

## Unreal 配置清单

### 1. 建立 Montage 资产

每个正式受击 `Sequence` 都要建立一个独立 `AnimMontage`，不要把 `Light`/`Heavy`/`GuardBreak` 全塞进同一个 Montage 里。

理由：

1. 便于按 `(HitReactionType, DirectionType)` 做稳定映射
2. 便于后续分别调 `Blend In/Out`、`Rate Scale`、通知点
3. 便于在调试信息里明确看到当前到底播的是哪一条

### 2. Slot 配置

本轮建议统一使用一个明确的全身受击 `Slot`，命名建议：

- `FullBody_HitReact`

你需要检查：

1. `AnimMontage` 里使用的 `Slot` 名称是否统一为 `FullBody_HitReact`
2. 玩家 `AnimBP` 的最终输出链路里，是否真的有对应 `Slot` 节点
3. `Slot` 节点位置是否在 Locomotion 状态机之后、最终输出之前，确保受击能盖住当前移动/攻击动作
4. `Slot Group` 是否没有被其它攻击 `Montage` 错误复用到会互相抢占的组

如果你项目里已经有现成的全身 `Slot`（例如 `FullBody`），本轮也可以直接复用；关键不是名字，而是“受击 Montage 与 AnimBP 里的 Slot 节点完全一致，并且能稳定全身覆盖”。

### 3. Notify 配置

本轮按“必须 / 建议预留”分两档：

必须检查：

1. 先确认当前实现是否已经监听某个 `Montage Notify`
2. 如果当前没有任何代码或蓝图在监听 `Notify`，那本轮不是必须靠 `Notify` 才能跑通，因为恢复逻辑仍可由既有时长驱动

建议预留：

1. `Montage Notify: HitReact_RecoveryWindow`
   - 放在你认为“角色已经完成主要受击、可以恢复控制/切回正常动作”的时间点
   - 这不是本轮硬依赖，但强烈建议先加，后续若恢复时机要从定时器切到更精确控制，就能直接用
2. `Montage Notify: HitReact_PresentationEnd`
   - 放在表现尾帧附近
   - 方便未来挂音效、镜头、调试或蓝图收尾

注意：

1. 如果后续实现明确要求监听 `Montage Notify`，一定要加在 `Montage` 上，而不是只在原始 `Sequence` 里放普通 `Anim Notify`
2. 本轮没有明确消费方时，不要为了“看起来完整”堆很多无用通知

### 4. Mapping 配置建议

建议你在角色蓝图或角色资源配置入口里按下面方式填：

1. `Light`
   - `Front` → `AM_HitReact_Light_F`
   - `Back` → `AM_HitReact_Light_B`
   - `Left` → `AM_HitReact_Light_L`
   - `Right` → `AM_HitReact_Light_R`
2. `Heavy`
   - `Front` → `AM_HitReact_Heavy_F`
   - `Back` → `AM_HitReact_Heavy_B`
   - `Left` → `AM_HitReact_Heavy_L`
   - `Right` → `AM_HitReact_Heavy_R`
3. `GuardBreak`
   - `Front` → `AM_HitReact_GuardBreak`
   - `Back` → `AM_HitReact_GuardBreak`
   - `Left` → `AM_HitReact_GuardBreak`
   - `Right` → `AM_HitReact_GuardBreak`
   - `Fallback` → `AM_HitReact_GuardBreak`

如果配置入口支持 `Fallback`，那么 `GuardBreak` 最好显式填上 `Fallback`；这样后续补四向资源时不会影响当前最小闭环。

### 5. Montage 自身参数检查

每条受击 `Montage` 至少检查这些项：

1. Skeleton 必须和玩家当前骨架一致
2. `Enable Auto Blend Out` 保持开启
3. `Blend In` 不要过长，避免受击进场发软
4. `Blend Out` 不要过长，避免逻辑恢复后表现还拖尾太久
5. Root Motion 如非明确需要，先保持关闭或与角色当前移动策略一致
6. Section 至少有默认段，避免后续播放入口混乱

### 6. 联调时该重点看的现象

1. 轻受击命中时，四个方向是否都能播到对应 `Light` 资源
2. 重受击命中时，是否确实换成 `Hit_Large_Combat_*`
3. `GuardBreak` 是否能稳定播到 `Block_Hit_Break_Seq` 制作的 Montage
4. 恢复后是否会残留 Montage 卡住、重复叠播或被错误循环
5. 角色正在攻击/移动时，受击是否能稳定全身接管，而不是只上半身抽动

## 本轮对程序侧的最小配合假设

1. 程序真相源继续使用现有 `HitReactionType + DirectionType`
2. 角色层只需要能按上述映射取到对应 `Montage`
3. 恢复时机第一版仍允许沿用现有最小时长 / Montage 时长策略
4. 若未来恢复点要精确到动画关键帧，再消费 `HitReact_RecoveryWindow`

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

## 策划 / 动画配置清单（摘要）

### 必需配置

1. 配置位置：玩家角色蓝图（角色资源配置入口，通常是角色派生 `Blueprint`）
   - 需要资产/数据：
     - `Light_Front/Back/Left/Right`
     - `Heavy_Front/Back/Left/Right`
     - `GuardBreak_Front/Back/Left/Right`
     - `GuardBreak_Fallback`
   - 用途：把 `HitReactionType + DirectionType` 正式映射到玩家受击表现资源
   - 备注：本轮 `GuardBreak` 允许四向共用同一条 `Montage`

2. 配置位置：玩家 `AnimBP`
   - 需要资产/数据：受击播放使用的 `Slot`、混合规则、必要时的全身/上半身覆盖策略
   - 用途：确保受击 `Montage` 能稳定播放并正确盖住当前动作
   - 备注：若本轮受击必须强打断，建议先走全身明确覆盖，后续再细化混合

3. 配置位置：受击 `Montage` 资源自身
   - 需要资产/数据：正确的段长、必要的 `Slot`、建议预留的 `Montage Notify`
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
