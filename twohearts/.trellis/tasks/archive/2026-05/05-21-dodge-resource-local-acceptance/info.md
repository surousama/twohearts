# 基础闪避资源配置与本地联调验收 - 实施记录

## 当前结论

1. `UTwoHeartsGA_Dodge` 主链路仍保持 `Root Motion + Notify` 驱动，没有回退到 Character 级位移逻辑。
2. `BP_ThirdPersonCharacter` 二进制引用中已能看到：
   `IA_Dodge`
   `IA_NormalAttack`
   `AM_Melee_NormalAttackCombo`
   `AS_Evade_Dodge_F / FR45 / R / BR45 / B / FL45 / L` 对应 Montage。
3. `IMC_Default` 已包含：
   `IA_Dodge`
   `IA_NormalAttack`
   `IA_Move`
   `IA_Look`
   `IA_Jump`
4. 本轮已补齐 Dodge 输入层调试记录：当 Dodge 因冷却、阻断 Tag、未绑定 Ability 或激活失败而没有真正进入 `UTwoHeartsGA_Dodge::ActivateAbility` 时，现在也会留下 `last_dodge_event` 与结构化日志。
5. 2026-05-21 已完成一次 `BuildEditor Development` 构建，结果通过。

## 本轮代码落点

1. `Source/twohearts/twoheartsCharacter.cpp`
   为 Dodge 输入补充 `InputScan / ActivateAbility / ActivateFailed / ForwardInputToActiveAbility` 调试事件与日志输出。
2. `.trellis/tasks/05-21-dodge-resource-local-acceptance/prd.md`
   补充当前阶段已确认的资源事实、验收口径和本地编辑器验收清单。

## 需要在 Unreal Editor 内继续确认的事项

1. 打开 `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`：
   确认 `DodgeConfig` 的 `Fallback / Forward / ForwardRight / Right / BackwardRight / Backward / BackwardLeft / Left / ForwardLeft` 是否按当前资源口径挂齐。
2. 打开每个 Dodge Montage：
   至少确认 `EnableRootMotion` 已开启；
   存在 `Dodge_InvulnerableBegin`；
   存在 `Dodge_InvulnerableEnd`；
   存在 `Dodge_Finished`。
3. 打开 `BP_ThirdPersonGameMode`：
   确认仍由 `ATwoheartsDebugHUD` 承担 HUD 调试显示。

## 本地联调操作口径

1. 启动 PIE 后先观察 HUD：
   `dodging`
   `direction`
   `invulnerable`
   `cooldown_ready`
   `last_dodge_event`
2. 在待机状态下测试：
   不给方向输入时应走 `Forward` 或明确的兜底方向；
   触发时 `dodging=YES`；
   动作结束后回到 `dodging=NO`；
   冷却结束后 `cooldown_ready=YES`。
3. 在移动状态下测试八向输入：
   方向命名与 Montage 选择应保持相对当前角色朝向；
   不应回归“先转身再闪”；
   `last_dodge_event` 应能看到激活、方向解析和冷却变化。
4. 连续按 Dodge 观察失败口径：
   冷却中应留下 `ActivateFailed` 或对应失败细节；
   若 Ability 没有绑定或被阻断 Tag 卡住，也应在日志/HUD 中有可读原因。
5. 联调普攻打断链路：
   若在 `Recovery` 内被 Dodge 打断，应能看到普攻侧与 Dodge 侧各自的关键事件；
   若在不可打断阶段尝试 Dodge，应能从 `last_dodge_event` 或日志判断失败原因，而不是只剩一句泛化 warning。

## 当前已知限制

1. 当前会话只能完成代码、资产引用静态核对和构建验证，无法替代 Unreal Editor 内的真实白盒联调。
2. Montage 是否真正开启 `Root Motion`、Notify 时机是否正确，仍需要在编辑器资源面板中逐项确认。
3. 本 task 还不能直接宣布“阶段验收完成”；只有在上述本地联调项通过后，才适合把结论回写为“允许进入公共战斗语义层开工评估”。
