# 基础闪避资源配置与本地联调验收

## Goal

作为第二章基础战斗模块下的子 task，在基础闪避方向问题收口后，继续完成资源配置、本地联调、验收回写，把基础闪避推进到“当前阶段可稳定验收”的状态。

## What I already know

* `UTwoHeartsGA_Dodge` 的 `C++` 主体已接通
* 当前第二章正确顺序仍是：基础闪避正式落地收口 -> 公共战斗语义层 -> 最小预输入
* `05-19-dodge-second-pass-polish` 负责处理靶向移动下闪避方向与角色朝向冲突
* 仅解决方向冲突还不等于基础闪避已完成本阶段验收
* 当前正式口径已经收束为：`Root Motion + Notify` 主驱动，角色侧继续承担资源配置与调试承载
* 当前角色判定为“资深程序”，本轮以实现收口、调试口径补齐和验收回写为主
* `BP_ThirdPersonCharacter` 已引用 `IA_Dodge / IA_NormalAttack`、八向 Dodge Montage 和角色动画蓝图
* `IMC_Default` 已包含 `IA_Dodge / IA_NormalAttack / IA_Move / IA_Look / IA_Jump`，输入映射资源本身已存在
* 当前 `ATwoheartsDebugHUD` 已能观察 `dodging / direction / invulnerable / cooldown_ready`，但 Dodge 输入失败时缺少专门调试落点

## Requirements

* 以上游 `05-19-dodge-second-pass-polish` 的方向口径为前提继续推进
* 补齐或确认当前基础闪避的资源配置入口、方向 Montage 映射、`Root Motion` 主路径和关键 Notify 配置
* 完成本地联调，确认方向、位移、冷却、无敌帧、动作结束和调试输出都符合当前口径
* 若 Dodge 无法激活、被冷却阻断、资源缺失或 `Root Motion / Notify` 配置不满足要求，需在日志或 HUD 侧留下可直接判断的问题口径
* 本轮需把联调结论和已知限制回写到父 task 或上游主文档，不只停留在口头结论
* 若发现仍有阻断“阶段验收”的问题，应明确沉淀为新的问题结论或新的下拆子 task，而不是直接跳去做公共战斗语义层

## Acceptance Criteria

* [ ] 待机和移动状态下都能稳定触发正式闪避动作，而不只是出现打断日志
* [ ] 闪避方向、方向命名和方向资源选择与当前口径一致，不回归“先转身再闪”的旧问题
* [ ] `Root Motion`、关键 Notify、冷却和无敌帧状态在本地联调中表现正常，或对已知缺口有明确记录
* [ ] 调试观察口径可用于判断 `dodging / dodge_direction / dodge_invulnerable / dodge_cooldown_ready`
* [ ] 当 Dodge 输入未成功激活时，能够从 HUD 或日志快速判断是冷却、阻断 Tag、资源缺失还是 Montage/Notify 配置问题
* [ ] 本轮阶段结论已回写，能支持主程序继续判断是否进入公共战斗语义层

## Out of Scope

* 公共战斗语义层正式实现
* 完整预输入
* 受击、伤害、格挡联动
* 联机同步改造

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 上游问题收口 task：`05-19-dodge-second-pass-polish`
* 对应旧文档：`../docs/程序技术文档/a基础闪避正式落地技术文档.md`
* 本轮优先检查角色侧配置、`GA_Dodge` 资源承载、HUD/日志观察口径和本地白盒验收结论
* 当前代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`
* 当前资源落点：
  `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset`
  `Content/Input/IMC_Default.uasset`
  `Content/Input/Actions/IA_Dodge.uasset`
  `Content/Chinese_Warrior/Animations/Combat_Montage/AS_Evade_Dodge_*_Montage.uasset`
* 本地编辑器验收需要重点确认：
  `BP_ThirdPersonCharacter` 中 `DodgeConfig` 八向 Montage 是否已正确挂载；
  相关 Dodge Montage 是否启用 `Root Motion`；
  Montage 中是否存在 `Dodge_InvulnerableBegin / Dodge_InvulnerableEnd / Dodge_Finished` Notify；
  `BP_ThirdPersonGameMode` 仍使用 `ATwoheartsDebugHUD`
* 若本轮存在只能在 Unreal Editor 内完成的配置或白盒验收步骤，必须把操作入口和观察口径补回本 task 与父 task
