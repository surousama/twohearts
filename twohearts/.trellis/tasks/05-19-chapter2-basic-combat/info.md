# 第二章基础战斗阶段总览

## 用途

1. 本文档用于在 Trellis 中承接第二章基础战斗模块当前阶段的高价值实施现状。
2. 它不是长期稳定规则，而是当前阶段的“实施地图”和“阶段事实总览”。
3. 长期稳定规则优先写入 `.trellis/spec/`；当前阶段现状、关键接口、代码落点、技术债和阶段收口情况优先收束在这里。

## 当前阶段结论

1. 第二章基础战斗模块已经完成第一轮正式战斗底座搭建。
2. 普通攻击正式承载已经稳定落在 `UE5 + GAS + C++` 路线下。
3. `AtwoheartsCharacter` 当前只应继续承担 `ASC` 持有与初始化、输入转发、角色侧资源配置和调试承载。
4. 普攻 `1 / 2 / 3` 已形成单路径 `Gameplay Ability` 承载，不应再回退到旧 Character 普攻状态机。
5. 普攻阶段语义 `Startup / Active / Recovery / LogicEnded` 已建立。
6. 当前最小 Dodge 打断规则已接通，基础闪避正式落地的 `C++` 主体也已接通。
7. 基础闪避仍需继续完成资源配置、本地联调和阶段收口，当前还不适合直接跳到公共战斗语义层。

## 当前正确顺序

1. 基础闪避正式落地收口
2. 公共战斗语义层
3. 最小预输入
4. 受击与伤害
5. 格挡
6. 更完整的战斗公共规则收束

## 为什么当前不是先做公共语义层

1. 公共语义层适合在“多个动作已经稳定存在”之后统一抽象，不适合在“普攻 + 验证型 Dodge”阶段提前抽。
2. 正式闪避会带来方向、位移、冷却、无敌帧、状态限制、动作结束衔接等真实约束；这些约束未稳定前，公共层很难一次抽准。
3. 最小预输入依赖统一动作上下文、逻辑结束事件和输入评估，因此也不适合排在公共语义层之前。

## 当前有效技术结构

1. 技术栈沿用 `UE5 + GAS + C++`。
2. 角色侧负责：
   `ASC` 持有与初始化；
   默认 Ability 授予；
   输入转发；
   资源配置入口；
   调试快照承载与显示。
3. 普攻 Ability 侧负责：
   段序进入；
   当前段 Montage 播放；
   段内再次输入接收；
   下一段推进；
   连段结束清理；
   普攻阶段维护；
   最小 Dodge 打断判定与执行；
   调试日志输出。
4. 正式基础 Dodge Ability 侧负责：
   闪避激活条件；
   方向解析；
   正式闪避生命周期；
   冷却；
   无敌帧状态；
   正式打断衔接；
   调试日志与调试字段。

## 当前关键接口现状

1. GAS 底座：
   `AtwoheartsCharacter` 已实现 `IAbilitySystemInterface`；
   角色已持有并初始化 `AbilitySystemComponent`；
   默认战斗 Ability 授予链路已打通。
2. 普攻输入入口：
   `AtwoheartsCharacter::NormalAttack`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
3. 普攻正式执行入口：
   `UTwoHeartsGA_NormalAttackBase::ActivateAbility`
   `UTwoHeartsGA_NormalAttackBase::InputPressed`
   `UTwoHeartsGA_NormalAttackBase::FinishSegment`
4. 普攻阶段入口：
   `UTwoHeartsGA_NormalAttackBase::EnterCombatPhase`
   `UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin`
   `UTwoHeartsGA_NormalAttackBase::NotifyCombatPhaseByName`
5. Dodge 打断与执行入口：
   `UTwoHeartsGA_NormalAttackBase::CanBeInterruptedByDodge`
   `UTwoHeartsGA_NormalAttackBase::TryInterruptByDodge`
   `UTwoHeartsGA_Dodge::ActivateAbility`
6. 调试状态入口：
   `AtwoheartsCharacter::SetNormalAttackDebugRuntimeState`
   `AtwoheartsCharacter::DrawNormalAttackDebugOverlay`
   `ATwoheartsDebugHUD::DrawHUD`

## 当前使用中的关键 Tag

1. 普攻相关：
   `Ability.NormalAttack`
   `Ability.NormalAttack.Segment1`
   `Ability.NormalAttack.Segment2`
   `Ability.NormalAttack.Segment3`
   `State.Action.NormalAttack`
2. 输入与动作限制相关：
   `State.CannotAttack`
   `State.CannotInput`
3. 阶段命名方向：
   `State.Phase.Startup`
   `State.Phase.Active`
   `State.Phase.Recovery`
   `State.Phase.LogicEnded`
4. Dodge 相关：
   `Ability.Dodge`
   `State.Action.Dodge`
   `State.CannotDodge`
   `State.Dodge.Invulnerable`
   `Cooldown.Dodge`

## 当前正式口径

1. 普攻首击只允许进入第 `1` 段。
2. 第 `1` 段和第 `2` 段执行期间再次点击，只缓存一次“下一段请求”。
3. 当前段结束后，如已请求下一段，则按 Tag 激活下一段 Ability。
4. 第 `3` 段结束后直接结束整套普攻。
5. `LogicEnded` 表示战斗规则层面的逻辑结束，不等同于 Montage 完全播完。
6. 基础闪避正式版按 `Root Motion` 承载。
7. 无敌帧开始、无敌帧结束和动作逻辑结束优先由 Montage Notify 驱动，配置时间窗仅作为最小兜底。
8. 普攻 `Recovery` 允许被闪避打断；`Startup / Active` 默认不允许被闪避直接打断。

## 当前实际代码落点

1. 角色入口与调试承载：
   `twohearts/Source/twohearts/twoheartsCharacter.h`
   `twohearts/Source/twohearts/twoheartsCharacter.cpp`
   `twohearts/Source/twohearts/twoheartsDebugHUD.cpp`
2. 普攻阶段枚举：
   `twohearts/Source/twohearts/TwoHearts/Combat/TwoHeartsCombatPhase.h`
3. 普攻 Ability 基类：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.cpp`
4. 三段正式普攻 Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.cpp`
5. 正式基础 Dodge Ability：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp`
6. 基础 Ability 与 Tag 出口：
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.cpp`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
   `twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.cpp`

## 已退出正式路径的旧内容

1. 已删除测试 Ability：
   `UTwoHeartsGA_TestNormalAttack`
2. 已退出正式路径的旧 Character 普攻函数：
   `TryStartNormalAttack`
   `PlayNormalAttackSegment`
   `HandleNormalAttackSegmentFinished`
   `ResetNormalAttackCombo`
3. 已删除旧 Character 普攻运行态：
   `bIsNormalAttacking`
   `CurrentNormalAttackSegment`
   `bHasQueuedNextNormalAttackSegment`
   `NormalAttackSegmentTimerHandle`
4. 已删除新旧切换开关：
   `bUseAbilitySystemForNormalAttackInput`

## 当前未完成项与技术债

1. 基础闪避第二轮本地联调验收尚未完成。
2. 正式通用预输入尚未完成。
3. 公共战斗语义层尚未完成。
4. 受击、伤害、格挡联动尚未完成。
5. 联机同步尚未处理。
6. Ability 结束与 Montage 收尾的一致性，后续仍需在公共语义层阶段继续统一。
7. `ActorInfo` 初始化与默认 Ability 授予链路，对未来双人共斗和联机扩展的鲁棒性，后续仍需复查。

## 来源

1. `../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
2. `../docs/程序技术文档/a基础闪避正式落地技术文档.md`
3. `../docs/测试文档/a普通攻击切换到AbilitySystem跑测清单.md`
4. `../docs/测试文档/a普通攻击测试调试功能说明.md`
