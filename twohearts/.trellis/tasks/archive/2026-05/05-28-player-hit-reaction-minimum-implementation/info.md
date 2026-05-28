# 玩家受击状态最小实现记录

## 本轮实际完成

1. 在公共动作上下文中新增 `HitReaction` 动作类型，受击现在是正式动作状态，不再只是日志提示。
2. 在 `UTwoHeartsHostileAttackReceiverComponent` 中新增最小受击状态承载：
   受击是否激活；
   受击来源；
   对应攻击实例；
   受击方向分类；
   受击开始时间；
   预计恢复时间；
   最后结束原因。
3. 当 `PlayerDamageResult` 生成 `DamageApplied` 时：
   会先明确打断当前动作；
   清理当前缓冲输入；
   正式进入受击状态；
   启动自动恢复计时器。
4. 当当前最小 Guard 将结果改写为 `GuardBlocked` 时：
   若本次攻击已触发受击，会同步结束这次受击状态，避免“格挡成功但仍停留在受击中”。
5. 扩展了正常攻击、闪避、Guard 的最小“被受击打断”路径，并把结束原因正式写回动作上下文。
6. 扩展 HUD，可观察：
   当前是否处于受击；
   受击来源攻击；
   受击类型；
   方向分类；
   开始时间 / 恢复时间；
   最后结束原因。

## 代码落点

1. `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h`
2. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
3. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
4. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
5. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
6. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
7. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
8. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h`
9. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.cpp`
10. `Source/twohearts/twoheartsDebugHUD.cpp`

## 当前规则口径

1. 受击期间，公共动作上下文会把当前动作类型切到 `HitReaction`。
2. 当前最小受击方向先采用四向分类：
   `Front / Back / Left / Right`
3. 当前最小时长规则：
   `Light = 0.35s`
   `Heavy = 0.50s`
   `GuardBreak = 0.65s`
4. 当前受击结束原因至少可区分：
   `AutoRecovered`
   `GuardBlocked`
   `SupersededByNewHit`

## 已知限制

1. 当前仍未接入正式受击动画资源或 Montage。
2. 当前方向只做到稳定数据承载和四向分类，没有扩展到完整四向 / 八向动画库。
3. 当前对更复杂动作集合的统一中断规则还未扩展到未来更多 Ability。
4. 当前恢复仍是固定时长计时器，不是资源驱动或状态机驱动。

## 本轮验证

1. 已执行 Unreal 编译：
   `H:\UE_5.6\Engine\Build\BatchFiles\Build.bat twoheartsEditor Win64 Development -Project='H:\twohearts\twohearts\twohearts.uproject' -WaitMutex -NoHotReload`
2. 编译结果：
   `Result: Succeeded`
3. 已完成最新一轮 PIE 跑测并检查 `Saved/Logs/twohearts.log`。
4. 日志确认连续正面受击时，`HostileProbe_1` 到 `HostileProbe_10` 的 `HitReaction event=Enter` 全部稳定记录为 `direction=Front`。
5. 本轮验证说明受击方向分类已修正为“玩家面向攻击来源”的语义，不再出现正面受击被记成 `Back` 的错误。
6. 日志确认 `HostileProbe_11` 之后只剩 `IgnoredNoHealth`，未再出现新的 `HitReaction event=Enter`，说明 `0` 血后不会继续进入新受击。

## 收口结论

1. 最小受击状态链路已可稳定覆盖：
   命中进入受击
   正面方向分类
   自动恢复
   `0` 血后停止继续进入新受击
2. 本轮针对正面受击方向误判的修复已通过日志验证。
