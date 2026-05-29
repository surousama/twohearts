# 跑测说明

## 当前实现范围

- 当前只覆盖 `ATwoHeartsHostileAttackProbeCharacter` 这一最小敌方样本
- 当前目标是验证“玩家普通攻击命中敌方 -> 敌方生成命中结果 -> 扣减生命 -> `0` 血停机 -> 后续新受击忽略”闭环
- 当前不验证完整敌方 AI、死亡演出、掉落、联机同步

## 你在编辑器里怎么跑

1. 打开包含玩家角色与 `HostileAttackProbe` 的测试地图
2. 进入 PIE
3. 让玩家靠近 `HostileAttackProbe`
4. 使用普通攻击连续命中它，直到生命值归零
5. 若需要验证 `0` 血后忽略口径：
   让角色继续对已经归零的 `HostileAttackProbe` 挥刀，观察是否仍有新日志

## 跑测时你应重点观察

1. 每次命中是否都能看到敌方收到玩家攻击信号
2. 日志里的 `attack=` 是否能区分当前是第几段普通攻击实例
3. 每次扣血后 `health=旧值->新值` 是否连续下降
4. 降到 `0` 血时是否出现停机日志
5. `0` 血后再次攻击时，是否出现 `IgnoredNoHealth` / `ignored at 0 hp` 一类日志，而不是继续扣血

## 我会读取的日志文件

- 主日志文件：`Saved/Logs/twohearts.log`
- 如果编辑器轮转日志，也可能读取：`Saved/Logs/twohearts-backup-*.log`

## 建议你跑完后告诉我的一句话

- 可以直接回复：
  `我跑完了，你看日志`

## 我会重点搜索的关键日志

- `[PlayerAttackSignal]`
- `[HostileHitResult]`
- `[HostileDamageResult]`
- `event=PlayerAttackSignalReceived`
- `event=PlayerHitResolved`
- `event=PlayerDamageResolved`
- `event=ZeroHealthShutdown`
- `ignored at 0 hp`
- `will_ignore_new_hits=true`

## 预期通过口径

- 至少出现一次 `HitConfirmed`
- 至少出现一次 `DamageApplied`
- 至少出现一次 `health=...->0.00` 或等价归零日志
- 归零时出现 `ZeroHealthShutdown`
- 归零后再次攻击时出现 `IgnoredNoHealth` 或等价忽略日志

## 当前补充说明

- 屏幕调试也会显示：
  `[Probe Hurt] <attack> -<damage> => <health>`
- 归零时会显示：
  `[Probe] defeated by <attack>`
- 这些屏幕字样只是辅助，最终验收以日志为准

## 实际实现回写

- 敌方最小生命承载落在 `UTwoHeartsPlayerAttackReceiverComponent`
- 组件现在会把玩家普攻信号正式收束为：
  `FTwoHeartsHostileHitResult`
  `FTwoHeartsHostileDamageResult`
- `ATwoHeartsHostileAttackProbeCharacter` 负责订阅这些结果，并输出敌方样本级调试日志与 `0` 血停机处理
- `0` 血后当前样本会停止自身攻击循环，并关闭触发与命中球碰撞，后续玩家命中会继续留下 `IgnoredNoHealth` 日志，但不会继续扣血

## 本轮跑测结论

- 跑测日志文件：`Saved/Logs/twohearts.log`
- 初始生命日志确认：
  `event=HealthInitialized current=30.00 max=30.00 defeated=false`
- 玩家普攻三段分别把生命从：
  `30 -> 20`
  `20 -> 10`
  `10 -> 0`
- 归零时出现：
  `event=ZeroHealthShutdown ... will_ignore_new_hits=true`
- 归零后继续攻击时，多次出现：
  `result=IgnoredNoHealth`
  `damage=0.00 health=0.00->0.00`

## 已知限制

- 当前只验证了单个 `HostileAttackProbe` 样本
- 当前没有接入完整敌方死亡演出、掉落或更通用的敌方属性平台
- 当前停机口径是最小闭环级别，后续完整敌人体系仍需要在正式属性/死亡系统里继续收束
