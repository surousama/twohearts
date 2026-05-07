# Lyra 移动系统实现参考

## 1. Pivot 的整体结构

Lyra 的 Pivot 分为两层：

1. **高层状态机 `LocomotionSM`**
   - 负责决定何时进入 `Pivot`
   - 负责 `Pivot -> Cycle` 的退出规则
   - 进入 `Pivot` 时记录本次 Pivot 的初始方向
   - 维护 `Last Pivot Time` 倒计时，防止 Pivot 刚进入就退出

2. **下层动画层 `FullBody_PivotState`**
   - 负责真正播放 Pivot 动画
   - 使用 `Sequence Evaluator + OnBecomeRelevant + OnUpdate`
   - 内部还有 `PivotA / PivotB` 两个状态，专门处理连续 Pivot 时的重入问题

## 2. 高层 Pivot 状态

### 2.1 On Become Relevant

高层 `SetUpPivotState` 只做一件事：

- `Pivot Initial Direction = Local Velocity Direction`

作用：
- 记录进入 Pivot 时的初始移动方向
- 供 `Is Moving Perpendicular to Initial Pivot` 判断使用

### 2.2 On Update

高层 `UpdatePivotState` 只做一件事：

- 如果 `Last Pivot Time > 0`，则每帧 `Last Pivot Time -= DeltaTime`

作用：
- 给 Pivot 一个最短有效持续时间
- 防止刚进入 Pivot 就因为输入变化或其它条件立刻退出
- `Last Pivot Time` 不是动画剩余时长，而是 Pivot 的**早期保护窗口/纠偏窗口**
- 这段时间内：
  - 高层退出条件不会太早放行
  - 下层允许重新修正所选 Pivot 动画

## 3. Pivot -> Cycle 退出条件

Lyra 使用 3 条转换线，按优先级从高到低：

1. **Linked Layer Changed**
   - 动画层切换时强制退出
   - 我们当前可忽略

2. **ANS：`TransitionToLocomotion`**
   - `Was Anim Notify State Active in Source State (Pivot)`
   - 作用不是“播完退出”，而是“播到允许退出的时间段后才退出”

3. **复合兜底条件**

```text
OR(
    OR(Crouch State Change, ADSState Changed),
    AND(
        Is Moving Perpendicular to Initial Pivot,
        Last Pivot Time <= 0.0
    )
)
```

对我们最有价值的点：

- Lyra 不只靠 ANS 退出
- 还额外要求 `Last Pivot Time` 已经耗尽
- 这样可以防止 Pivot 太早退出

## 4. FullBody_PivotState 结构

`FullBody_PivotState` 不是单一动画节点，而是：

1. 一个内部状态机 `PivotSM`
2. 一个单独的 `Sequence Evaluator`
3. 通过 `Layered Blend per Bone` 混合

其中：

- `PivotSM` 负责下半身/主体 Pivot
- 额外的 `Sequence Evaluator` 用于上半身 HipFire 覆盖
- 最终再叠加 `Orientation Warping` 和 `Stride Warping`

## 5. PivotSM：为什么有 PivotA / PivotB

Lyra 用 `PivotA` 和 `PivotB` 两个状态交替承载 Pivot。

原因：

- UE 状态机对“同一 Pivot 状态的连续重入”支持不好
- 连续反向输入时，单个 Pivot 状态可能无法重新初始化
- 用两个状态轮流切，可以保证每次都重新触发 `OnBecomeRelevant`

这对我们有启发：

- 如果当前实现只有一个 Pivot 状态，连续反向或快速重入时更容易出问题

## 6. WantsToRePivot 规则

`WantsToRePivot` 的作用不是“进入 Pivot”，而是：

- 当前已经在 `PivotA` 时，是否切去 `PivotB`
- 当前已经在 `PivotB` 时，是否切去 `PivotA`

核心判断包含 3 点：

1. `LocalVelocity2D · LocalAcceleration2D < 0`
   - 当前仍然是反向制动语义

2. `NOT IsMovingPerpendicularToInitialPivot`
   - 当前意图还没有偏成横向

3. `LocalAcceleration2D · PivotStartingAcceleration < 0`
   - 当前加速度方向和进入 Pivot 时记录的加速度方向又发生反转

作用：

- 玩家在 Pivot 过程中再次反向输入时，允许重新起播一次 Pivot

## 7. PivotA / PivotB 的动画层实现

`PivotA`、`PivotB` 的核心结构一致：

```text
Sequence Evaluator
→ Orientation Warping
→ Stride Warping
→ Output Pose
```

并绑定：

- `On Become Relevant = SetUpPivotAnim`
- `On Update = UpdatePivotAnim`

## 8. SetUpPivotAnim 做了什么

Lyra 在进入 Pivot 动画时，会做以下初始化：

1. 记录 `Pivot Starting Acceleration`
2. `Convert to Sequence Evaluator`
3. 根据输入方向 `Get Desired Pivot Sequence`
4. `Set Sequence`
5. `Set Explicit Time(0.0)`
6. `Stride Warping Pivot Alpha = 0.0`
7. `Time at Pivot Stop = 0.0`
8. `Last Pivot Time = 0.2`

这里最关键的 3 点：

1. **明确切换到正确的 Pivot 动画**
2. **明确把 `Explicit Time` 重置到 0**
3. **明确给 `Last Pivot Time` 一个 0.2 秒保护时间**

变量语义补充：

- `Last Pivot Time`
  - 含义：本次 Pivot 还剩多少“早期保护时间”
  - 不是动画播放剩余时间
  - 主要用于防止刚进入 Pivot 就退出，并允许早期修正 Pivot 动画方向
- `Time at Pivot Stop`
  - 含义：角色到达 Pivot 点那一刻，对应的动画时间
  - 供后半段计算 `Stride Warping Pivot Alpha` 使用

## 9. UpdatePivotAnim 的核心逻辑

### 9.1 开头统一处理

每帧先：

1. `Convert to Sequence Evaluator`
2. 缓存当前 `Sequence Evaluator`
3. 读取当前 `Accumulated Time`
4. 回写 `Explicit Time`

说明：

- Lyra 的 Pivot 动画时间是“受控推进”的，不是完全放任自然播放

### 9.2 进入早期允许短时间切换 Pivot 动画

当 `Last Pivot Time > 0` 时：

1. 重新计算 `Desired Pivot Sequence`
2. 如果和当前序列不同，则：
   - `Set Sequence with Inertial Blending`
   - `Blend Time = 0.2`
   - 同时更新 `Pivot Starting Acceleration`

作用：

- Pivot 刚进入的短时间内，允许根据最新输入修正所选 Pivot 动画
- 不是一进来就把方向彻底锁死

### 9.3 速度与加速度反向时

当 `Velocity · Acceleration < 0` 时，说明角色仍在靠近 Pivot 点：

1. `Predict Ground Movement Pivot Location`
2. 取返回偏移向量的 `Vector Length XY`
3. `Distance Match to Target`
4. 并把当前 `Explicit Time` 记录到 `Time at Pivot Stop`

作用：

- 在“急停靠近 Pivot 点”的前半段，用距离匹配把动画准确对齐到回转点
- 顺便记录“到达 Pivot 点时动画播到了哪”，供后半段继续衔接

### 9.4 速度与加速度同向时

当 `Velocity · Acceleration >= 0` 时，说明角色已经过了 Pivot 点，开始反向加速：

1. 根据 `Explicit Time`、`Time at Pivot Stop` 计算 `Stride Warping Pivot Alpha`
2. 平滑提高最小 `Play Rate Clamp`
3. `Advance Time by Distance Matching`

作用：

- 后半段不再用 `Distance Match to Target`
- 改为按实际位移推进动画
- 同时逐步把 Stride Warping 混进来，减少后半段滑步

这两个变量的区别：

- `Last Pivot Time`
  - 管的是状态保护窗口
  - 决定 Pivot 早期能否退出、能否改判动画
- `Time at Pivot Stop`
  - 管的是动画时间分界点
  - 决定后半段从哪里开始混入 Warping

## 10. 对我们当前 debug 最有价值的结论

1. **Lyra 进入 Pivot 时一定会 `Set Explicit Time(0.0)`**
   - 如果我们当前实现没有稳定做到这一点，Pivot 可能一进入就从很后面开始

2. **Lyra 有 `Last Pivot Time = 0.2` 的保护**
   - 即使方向有变化，也不会让 Pivot 刚进来立刻退出

3. **Lyra 前半段和后半段不是同一种时间推进方式**
   - 前半段：`Distance Match to Target`
   - 后半段：`Advance Time by Distance Matching`

4. **Lyra 会记录 `Time at Pivot Stop`**
   - 用来控制后半段 `Stride Warping` 的混入时机

5. **Lyra 支持 Pivot 早期重新选动画**
   - 通过 `Set Sequence with Inertial Blending`
   - 可以修正刚进入 Pivot 时方向判断的轻微变化

6. **Lyra 使用 PivotA / PivotB 处理连续 Pivot**
   - 单个 Pivot 状态在快速重入时更不稳

## 11. 对我们当前 bug 的直接启发

对于“Pivot 只持续极短时间，然后很快进入 Cycle，并产生巨大形变”这个问题，优先对比以下几项：

1. `Pivot OnBecomeRelevant` 是否稳定执行了：
   - `Set Sequence`
   - `Set Explicit Time(0.0)`

2. 当前实现是否有类似 `Last Pivot Time = 0.2` 的最短保护时间

3. 当前实现是否像 Lyra 一样：
   - 前半段用 `Distance Match to Target`
   - 后半段改用 `Advance Time by Distance Matching`

4. 当前实现是否记录了类似 `Time at Pivot Stop` 的变量，用来决定后半段 Warping 混入时机

5. 当前实现是否只有单一 Pivot 状态，缺少 `PivotA / PivotB` 这种重入保护
