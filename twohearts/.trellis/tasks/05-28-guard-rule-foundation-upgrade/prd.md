# 格挡规则基础升级

## Goal

作为 `05-19-chapter2-basic-combat` 之下、建立在攻击元数据、正式伤害结果和玩家受击状态之上的执行子 task，把当前基础 Guard 从“最小窗口改写命中结果”升级为“按攻击段规则、时机和位置条件判定”的基础格挡规则系统，使格挡开始真正靠近原始框架文档定义，而不是只对单一样本成立。

## What I already know

* `05-25-basic-guard-implementation` 与 `05-25-guard-feedback-and-whitebox-validation` 已经证明基础 Guard 存在且可验收。
* 但当前 Guard 更接近“最小时间窗 + 对最小敌对攻击样本生效”的版本，还不是完整格挡规则系统。
* 原始框架文档明确要求：
  攻击段自身需要具备是否可格挡的配置；
  格挡成功要同时满足时机、位置、角度 / 距离等条件；
  可格挡与不可格挡攻击需要有不同提示语义。
* 因此，本 task 的重点是先把 Guard 的判定条件做正确，而不是马上堆上所有成功收益或表现演出。

## Requirements

* 在现有基础 Guard 之上，引入正式的攻击段格挡规则承载。
* 当前至少需要补齐：
  攻击段是否可格挡；
  最小格挡时机判断；
  最小位置 / 角度 / 距离条件；
  成功 / 失败 / 不可格挡的结果分支；
  最小攻击提示语义。
* 可格挡与不可格挡攻击当前允许先使用简化提示方式，但必须在规则层明确区分，而不是统一当成一个窗口。
* Guard 成功 / 失败必须继续沿用正式伤害结果与正式受击状态口径，不允许绕开它们另写平行处理。
* 当前仍允许以 `05-25-minimal-hostile-attack-probe` 为主验证样本，但设计上不能把规则写死成“只适配这一个样本”。
* 需要保留清晰调试输出，便于区分：
  这次攻击是否可格挡；
  当前失败是因为太早 / 太晚 / 方向错误 / 距离错误 / 不可格挡；
  当前成功是否进入了正式格挡分支。
* 本轮不应把任务扩大成完整弹反系统、完整镜头 / 特效表现、完整资源收益体系或完整多人联合格挡。

## Acceptance Criteria

* [ ] 当前已经存在正式的“攻击段格挡规则”而不是只有最小时间窗。
* [ ] 至少能够区分：可格挡攻击、不可格挡攻击、成功格挡、失败格挡。
* [ ] Guard 成功 / 失败继续沿用正式伤害与受击口径，没有另起平行命中系统。
* [ ] 最小位置 / 角度 / 距离条件已经存在，哪怕当前实现仍是简化版。
* [ ] 本轮没有越界扩散成完整弹反系统、完整重表现系统或完整数值收益系统。

## Out of Scope

* 完美格挡 / Parry 完整系统
* 完整格挡镜头、震屏、音效、特效打磨
* 格挡成功后的全部收益结算
* 联合格挡、多角色同步格挡
* 完整盾牌 / 装备体系

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-28-attack-metadata-foundation`
  `05-28-player-damage-result-formalization`
  `05-28-player-hit-reaction-minimum-implementation`
* 后置依赖：
  `05-28-guard-outcome-settlement-and-counter-hook`
* 当前建议优先查看的代码落点：
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
  `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
* 当前允许的过渡边界：
  允许先用单一敌对攻击样本完成规则闭环；
  允许提示先用简化调试或占位表现；
  但不允许继续把 Guard 成败完全写死在单一时间窗判断上。
