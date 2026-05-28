# 玩家伤害结果正式化实现记录

## 本轮实际完成

1. 在 `UTwoHeartsHostileAttackReceiverComponent` 上新增正式的 `FTwoHeartsPlayerDamageResult` 承载。
2. 在同一组件上新增最小血量承载：`MaxHealth=100`、`CurrentHealth`。
3. 将现有 `PlayerHitResult` 升级为正式伤害入口：
   `HitConfirmed` 会生成 `DamageApplied` 并扣减最小血量；
   `GuardRewritten` 会生成 `GuardBlocked`，并在已扣血时回滚本次伤害。
4. 在 `FTwoHeartsAttackMetadata` 中补入 `BaseDamage`，作为当前最小正式基础伤害字段。
5. 扩展调试 HUD 与日志，当前可直接观察：
   基础伤害；
   最终伤害；
   是否被 Guard 改写；
   当前剩余血量；
   伤害结果时间戳与前后血量。

## 代码落点

1. `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
2. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h`
3. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.cpp`
4. `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.cpp`
5. `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
6. `Source/twohearts/twoheartsDebugHUD.cpp`

## 当前规则口径

1. 当前最小正式伤害值直接来自 `AttackMetadata.BaseDamage`。
2. 当前敌对攻击探针与普攻模板都先使用 `10.0f` 作为最小默认基础伤害。
3. `HitExpired`、`SignalInvalid`、`PendingIncomingHit` 不会生成正式伤害结果。
4. `GuardRewritten` 不再只是改写命中结果，也会同步改写伤害结果与最小血量。

## 已知限制

1. 目前仍不是完整 `AttributeSet` / 属性系统。
2. 当前还没有正式治疗、减伤百分比、穿透、霸体或多来源结算规则。
3. `BaseDamage` 目前仍是最小占位值，后续应由更正式的攻击段规则或配置驱动。
4. 当前血量承载仍在 `HostileAttackReceiverComponent`，后续若引入正式属性层，需要再迁移真相源。

## 本轮验证

1. 已执行 Unreal 编译：
   `H:\UE_5.6\Engine\Build\BatchFiles\Build.bat twoheartsEditor Win64 Development -Project='H:\twohearts\twohearts\twohearts.uproject' -WaitMutex -NoHotReload`
2. 编译结果：
   `Result: Succeeded`
3. 已完成最新一轮 PIE 跑测并检查 `Saved/Logs/twohearts.log`。
4. 日志确认前 10 次正面命中持续产生 `DamageApplied`，血量按 `100 -> 0` 每次 `10` 点稳定递减。
5. 日志确认 `HostileProbe_10` 命中后 `health_after=0.00`。
6. 日志确认 `HostileProbe_11` 到 `HostileProbe_14` 均为 `IgnoredNoHealth`，`final=0.00`，且 `health_before/after` 持续保持 `0.00`。

## 收口结论

1. `PlayerDamageResult` 现已具备最小正式化承载能力，可稳定表达：
   `DamageApplied`
   `GuardBlocked`
   `IgnoredNoHealth`
2. `0` 血后不再继续扣血，后续命中会被正式记为 `IgnoredNoHealth`，本轮日志验证通过。
