# Lyra 移动系统实现参考

## Pivot 状态

### Pivot → Cycle 过渡条件（3个，按优先级排列）

Lyra 使用了 **3 条转换线** 从 Pivot 到 Cycle（通过 State Alias `CycleAlias` 汇聚），优先级从高到低：

---

#### 优先级 1：Linked Layer Changed

- **Details 面板设置**：Transition Rule Sharing = Use Shared → `Promote To Shared`
- **含义**：当 Animation Layer（动画层）发生切换时（比如角色切换了武器/姿态集，导致整个动画层被替换），强制退出当前状态回到 Cycle。这是一个"兜底重置"机制，确保动画层切换后不会卡在旧状态里。
- **对我们的影响**：我们当前没有使用 Linked Anim Layer 系统，**这个条件暂时不需要实现**。

---

#### 优先级 2：动画通知状态（ANS）

- **节点**：`Was Anim Notify State Active in Source State (Pivot)`
- **Anim Notify State Type**：`TransitionToLocomotion`（路径：`/Game/Characters/Heroes/Mannequin/Animations/AnimNotifies/TransitionToLocomotion`）
- **含义**：Pivot 动画上挂了一个名为 `TransitionToLocomotion` 的 ANS（动画通知状态）。这个 ANS 覆盖了动画的某个时间段（比如从第 0.3 秒到动画结束）。当 ANS 激活时（即动画播放到了被 ANS 覆盖的时间段），这个条件返回 true，允许过渡到 Cycle。
- **作用**：精确控制"Pivot 动画至少要播放多长时间才允许退出"。比如 ANS 从 0.3 秒开始，那前 0.3 秒是不允许退出的（保证回转动画的核心部分播完）。
- **对我们的影响**：**这是我们要实现的核心机制**，替代当前的"动画自然播放完成"退出方式。

---

#### 优先级 3：复合条件（兜底退出）

逻辑结构：
```
Can Enter Transition = OR(
    OR(Crouch State Change, ADSState Changed),
    AND(
        Is Moving Perpendicular to Initial Pivot,
        Last Pivot Time <= 0.0
    )
)
```

各子条件含义：

| 条件 | 含义 |
|------|------|
| **Crouch State Change** | 蹲伏状态发生变化（站↔蹲切换），强制退出 Pivot |
| **ADSState Changed** | 瞄准状态发生变化（进入/退出瞄准），强制退出 Pivot |
| **Is Moving Perpendicular to Initial Pivot** | 玩家当前的移动方向与 Pivot 初始方向垂直（即不再是反向输入，而是横向输入了），说明玩家意图已经变了 |
| **Last Pivot Time <= 0.0** | Pivot 计时器归零（Pivot 动画的有效时间已经耗尽）|

- **AND 的含义**：只有当"移动方向已经偏离了 Pivot 方向"**并且**"Pivot 时间已经结束"同时满足，才通过这个条件退出。
- **OR 的含义**：状态切换（蹲/瞄准）是高优先级打断，无需等待其他条件。
- **对我们的影响**：Crouch/ADS 我们暂时没有，但 `Is Moving Perpendicular to Initial Pivot` + `Last Pivot Time` 这个组合值得参考——它防止了"Pivot 刚进入就被横向输入踢出去"的问题。

---

### 状态别名（State Alias）使用

- Lyra 使用了 `CycleAlias` 作为 Cycle 状态的别名，多个状态（Pivot、Stop、Start 等）的出口转换线都指向这个别名，而不是直接指向 Cycle 状态节点。
- **好处**：减少连线混乱，所有"回到 Cycle"的转换只需要连到别名，修改 Cycle 状态时不影响转换线。

（待记录我们的实施方案）

