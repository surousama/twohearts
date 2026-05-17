# 基础闪避跑测+白盒反馈

## 1. 跑测问题

### 1.1 第一次闪避成功后，后续闪避持续被 cooldown 阻挡

1. 优先级：
   P0
2. 现象：
   第一次闪避正常。
   后续闪避持续被 `cooldown` 阻挡，游戏内有黄色日志提示。
3. 预期：
   冷却结束后应允许再次闪避。
4. 实际：
   冷却似乎没有正常结束，或冷却状态没有正常清理。

### 1.2 闪避瞬间剧烈位移，随后闪回原位置

1. 优先级：
   P0
2. 现象：
   闪避瞬间会被猛推到目标方向，随后直接闪回。
3. 预期：
   闪避应平稳完成位移，结束后停留在闪避后的位置。
4. 实际：
   当前位移和动作表现明显不一致。

### 1.3 闪避后角色从靶向移动变成朝向移动

1. 优先级：
   P1
2. 现象：
   闪避前是靶向移动，闪避后变成朝向移动。
3. 预期：
   闪避前后的移动模式应保持一致。
4. 实际：
   闪避动作改掉了角色移动/朝向相关状态，结束后没有正确恢复。

## 2. 白盒问题

### 2.1 冷却清理依赖 Ability 结束后的 ActorInfo，存在冷却永久不消失风险

1. 优先级：
   P0
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:293>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:304>)
   [TwoHeartsGameplayAbility.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.cpp:20>)
3. 问题说明：
   当前冷却用 `AddLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge)` 手动加。
   冷却结束时再通过 `HandleDodgeCooldownFinished -> ClearDodgeCooldown` 手动删。
   但 `ClearDodgeCooldown` 里重新通过 `GetTwoHeartsAbilitySystemComponent()` 取 ASC，这个入口依赖当前 Ability 的 `ActorInfo`。
   闪避 Ability 已经在 `FinishDodge -> EndAbility` 后结束，结束后这个回调是否还能稳定拿到有效 ASC，不可靠。
   一旦这里拿不到 ASC，冷却 Tag 就删不掉，现象就会变成“第一次闪避后永久被 cooldown 阻挡”。
4. 和跑测现象的对应：
   直接对应问题 `1.1`。
5. 建议修改方式：
   不要把正式冷却清理绑定在 Ability 结束后的实例回调里再临时删 Loose Tag。
   优先改成标准 GAS 冷却做法，用 `Cooldown GameplayEffect` 承载。
   如果本轮先不接 GE，至少要把 ASC 缓存在闪避开始时，冷却结束时直接用缓存的 ASC 清理 Tag，不要再依赖 Ability 结束后的 `CurrentActorInfo`。
   另外建议把“加冷却”和“清冷却”的日志补成一组，便于确认冷却生命周期有没有真的走完。

### 2.2 位移承载和动画承载完全分离，容易出现猛冲、闪回、动作不同步

1. 优先级：
   P0
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:168>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:174>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:185>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:394>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:264>)
3. 问题说明：
   当前闪避位移完全靠定时器里 `SetActorLocation` 硬推。
   动画完全靠 `AnimInstance->Montage_Play` 直接播。
   两边没有统一承载，也没有共享结束条件。
   这意味着：
   动画长度、Root Motion、位移距离、结束时机只要有一个不一致，就会出现视觉和胶囊不同步。
   当前结束时还会直接 `Montage_Stop(0.08f)`，更容易把问题放大成猛冲、抽搐、闪回。
4. 和跑测现象的对应：
   直接对应问题 `1.2`。
5. 建议修改方式：
   位移和动作要统一承载，至少二选一，不要一边手推位置一边裸播 Montage。
   如果这一版继续走非 Root Motion 方案，就让 C++ 位移做主，动画只做纯表现，并确保资源侧没有额外位移。
   如果这一版准备走 Root Motion 方案，就不要再用 `SetActorLocation` 每帧硬推。
   更稳的做法是改成 AbilityTask 或 CharacterMovement/RootMotionSource 一套承载到底，不要让“动画结束”和“位移结束”分别各管一套。

### 2.3 闪避强写角色朝向和旋转模式，没有恢复进入闪避前的原状态

1. 优先级：
   P1
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:165>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:168>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:257>)
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:38>)
3. 问题说明：
   闪避开始时直接把 `bOrientRotationToMovement` 改成 `false`，并直接 `SetActorRotation(DodgeDirection.Rotation())`。
   闪避结束时又直接把 `bOrientRotationToMovement` 写死成 `true`。
   这里没有记录“闪避前原本是什么状态”，也没有恢复 `bUseControllerRotationYaw`、`UseControllerDesiredRotation` 这类可能由蓝图或锁定系统控制的状态。
   所以只要角色本来不是这套默认朝向逻辑，闪避结束后就很容易被改成另一套移动模式。
4. 和跑测现象的对应：
   直接对应问题 `1.3`。
5. 建议修改方式：
   闪避开始前先缓存角色原本的旋转/移动相关状态。
   闪避结束时按缓存恢复，不要写死恢复成 `true`。
   如果项目里有靶向移动 / 非靶向移动两套模式，闪避不应该直接改最终模式，只能临时借用，结束后必须完整归还。

### 2.4 非正常结束路径不会做完整收尾，后续还会继续污染角色状态

1. 优先级：
   P1
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:71>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:242>)
3. 问题说明：
   当前只有 `FinishDodge` 会恢复旋转模式、停 Montage、收尾状态。
   但 `EndAbility` 本身没有做这些收尾。
   这意味着只要闪避被外部取消、未来被更高优先级动作中断，或者后续改逻辑时直接走了 `EndAbility`，角色就可能残留错误朝向、错误动作态、错误表现。
4. 当前影响：
   这个问题本轮不一定每次都能跑出来，但会让闪避后续很难扩展，也会让 `1.3` 这类问题更容易复发。
5. 建议修改方式：
   把“恢复角色状态”的逻辑收束到统一的收尾函数里。
   无论自然结束还是取消结束，都走同一套收尾，不要让 `FinishDodge` 和 `EndAbility` 各管一半。

### 2.5 移动输入缓存没有清零，停手后闪避仍可能读到旧方向

1. 优先级：
   P2
2. 问题定位：
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:126>)
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:373>)
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:445>)
3. 问题说明：
   `CachedMoveInput` 只在 `MoveAction Triggered` 时更新。
   当前没有看到在松开移动输入时把它清零。
   这样角色即使已经停下，只要上一次移动输入不是零，`GetDesiredDodgeDirectionWorld` 仍会优先按旧输入方向判定闪避，不会走“无输入时按面朝方向闪避”的口径。
4. 当前影响：
   这会让“站定闪避方向不对”变成隐性问题，和文档规则不一致。
5. 建议修改方式：
   给移动输入补 `Completed` 或等效清零逻辑。
   或者不要只读缓存值，改成读取当前真实输入状态/当前速度状态后再决定是否按面朝方向兜底。

## 3. 建议程序先修顺序

1. 先修 `2.1`，这是当前最阻断的，直接导致闪避只能成功一次。
2. 再修 `2.2`，这是当前动作表现最错的部分，不修掉后面的动作层验收没意义。
3. 再修 `2.3` 和 `2.4`，把移动模式和收尾逻辑收干净。
4. 最后补 `2.5`，避免后面联调时又冒出方向判定问题。

## 4. 复测重点

1. 冷却结束后，连续多次闪避是否都能恢复可用。
2. 闪避过程中角色是否还会猛冲后闪回。
3. 闪避结束后是否仍保持原本的靶向移动模式。
4. 停止移动后再按闪避，是否会错误沿上一次移动方向闪避。
