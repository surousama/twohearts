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

## 当前主程序评估

1. 当前父 task 仍处在“基础闪避正式落地收口”阶段，还没有进入“公共战斗语义层正式开工”。
2. 原因不是主顺序变化，而是当前进行中的 `05-19-dodge-second-pass-polish` 只覆盖了闪避方向与朝向冲突，不覆盖资源配置、本地联调、阶段验收回写。
3. 因此，“方向问题修完”只能说明基础闪避离稳定更近一步，不能单独作为切换到公共战斗语义层的开工信号。
4. 当前更合理的主程序推进方式是：
   先完成 `05-19-dodge-second-pass-polish`；
   再完成资源配置与本地联调验收；
   验收通过后再做一次公共战斗语义层开工评估与实施拆分。
5. 当前不建议提前拆“最小预输入”子 task，因为它仍然依赖公共战斗语义层先稳定。

## 当前建议子 task 顺序

1. `05-19-dodge-second-pass-polish`
   目标：
   收口靶向移动下闪避方向与角色朝向冲突，统一当前闪避方向口径。
2. `05-21-dodge-resource-local-acceptance`
   目标：
   在方向问题收口后，继续补齐 `Dodge Montage / Root Motion / Notify / 角色配置 / 本地联调 / 验收回写`，把基础闪避真正推到“当前阶段可验收”。
3. `05-21-combat-semantic-layer-readiness`
   目标：
   站在主程序视角复核“普攻 + 基础闪避”是否已经形成足够稳定的共享概念，并产出可直接派给资深程序的公共战斗语义层实施拆分。

## 当前阶段入口判断

1. 若 `05-19-dodge-second-pass-polish` 尚未完成：
   当前主入口仍是它，不切阶段。
2. 若 `05-19-dodge-second-pass-polish` 已完成，但 `05-21-dodge-resource-local-acceptance` 未完成：
   当前仍视为基础闪避阶段内部推进，不进入公共战斗语义层。
3. 只有当 `05-21-dodge-resource-local-acceptance` 明确给出“方向 / 位移 / 冷却 / 无敌帧 / 调试口径 / 本地联调”通过结论后，才允许启动 `05-21-combat-semantic-layer-readiness`。

## 来源

1. `../docs/程序技术文档/a第二章基础战斗模块开发总文档.md`
2. `../docs/程序技术文档/a基础闪避正式落地技术文档.md`
3. `../docs/测试文档/a普通攻击切换到AbilitySystem跑测清单.md`
4. `../docs/测试文档/a普通攻击测试调试功能说明.md`

## 2026-05-23 公共语义层 readiness 更新

1. `05-21-combat-semantic-layer-readiness` 已进入主程序正式评估阶段。
2. 当前阶段结论更新为：
   只要 `05-21-dodge-resource-local-acceptance` 对应的 Unreal Editor 白盒验收已经实际完成并通过，就允许进入“公共战斗语义层”首轮实施。
3. 当前不建议把“公共语义层”理解成一次性做完整战斗中台；第一轮只应先收束：
   动作上下文；
   阶段语义；
   逻辑结束出口；
   普攻与 Dodge 的统一打断接入口。
4. 当前不应直接跳到“最小预输入”。
   原因：
   输入评估与缓存执行仍依赖公共动作上下文先稳定。
5. readiness 后的建议顺序更新为：
   动作上下文底座
   -> 普攻语义桥接
   -> Dodge 语义桥接与打断统一
   -> 输入评估与预输入预留接口
6. 该顺序视为当前阶段固定顺序，不建议并行打乱：
   `combat-action-context-foundation`
   -> `normal-attack-semantic-bridge`
   -> `dodge-semantic-bridge-and-interrupt-unification`
   -> `combat-input-evaluation-preinput-hook`
