# 完整格挡表现实现

## Goal

作为 `05-19-chapter2-basic-combat` 之下的新执行子 task，把当前“Guard 已有规则、结算、HUD 与 DrawDebug，但还没有正式表现链”的状态升级为完整格挡表现实现，让格挡从工程验证能力变成玩家可直接感知的正式战斗表现能力。

## What I already know

* 当前 Guard 逻辑层已经较完整：
  输入；
  生命周期；
  规则判定；
  结果结算；
  调试 HUD；
  DrawDebug
* 当前 Guard 明确缺失正式表现层：
  没有成体系的格挡美术资产配置；
  没有正式格挡音效；
  没有正式 VFX / 相机 / 时间 / UI 反馈
* 当前仓库 `Content/` 中没有命名明确、已接线的 Guard 专用资源链；代码也未接入 `Sound/Niagara/CameraShake/Widget` 等正式反馈点
* 本 task 必须建立在 Guard 结果分支稳定的前提下，尤其需要 `不可格挡` 与 `GuardBreak` 结果已具备正式规则输入

## Requirements

* 必须实现完整格挡表现，而不是继续只依赖 `HUD + 日志 + DrawDebug`
* 本 task 必须至少覆盖以下内容：
  格挡相关美术资产配置；
  格挡音效；
  格挡反馈
* 上述“格挡反馈”当前至少要包含：
  Guard 启动反馈；
  Guard 成功反馈；
  Guard 失败反馈；
  不可格挡反馈；
  GuardBreak 反馈
* 当前允许的反馈形式包括但不限于：
  Montage / 动画状态；
  Niagara / 粒子；
  音效；
  相机震动；
  HitStop / 时间缩放；
  HUD / 屏幕提示
* 必须把资源配置入口正式落到蓝图 / 数据配置或等价可维护位置，不允许把所有资源路径硬编码在 `C++` 里
* 必须保持与现有 Guard 规则、GuardOutcome、HitReactionState 一致，不另起一套表现侧真相源

## Acceptance Criteria

* [ ] Guard 成功、普通失败、不可格挡、GuardBreak 至少都有可区分的正式表现
* [ ] 已存在正式的格挡美术资产配置入口，而不是只靠临时硬编码或调试几何
* [ ] 已接入正式格挡音效，并能在关键 Guard 结果分支上稳定触发
* [ ] 已接入至少一组正式格挡反馈手段，且在 PIE 中可稳定观察
* [ ] 表现触发与当前 Guard 逻辑结果保持一致，不出现结果分支正确但表现串线
* [ ] 本轮没有越界扩散到完整 UI 系统重构或所有未来角色的一次性资源通配

## Out of Scope

* 全角色、全武器、全敌人一次性适配
* 完整战斗 UI 系统重构
* 完整防御资源系统设计
* 网络同步与远端表现复制

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-28-guard-unguardable-and-guardbreak-rules`
  `05-28-player-hitreaction-presentation-formalization`
* 关键代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
* 关键资源与配置排查入口：
  `Content/ThirdPerson/Blueprints/`
  `Content/Chinese_Warrior/Animations/`
  `Content/Input/Actions/IA_Guard.uasset`
* 当前主程序判断：
  本 task 的重点是“把 Guard 结果接成正式表现”
  不是重新定义 Guard 规则本身

