# 基础格挡正式实现

## Goal

作为 `05-19-chapter2-basic-combat` 之下、建立在“最小敌对攻击探针”和“玩家受击与伤害最小闭环”之上的执行子 task，正式把 Guard 接入当前 `UE5 + GAS + C++` 基础战斗架构，使玩家第一次具备可触发、可判定、可改写来袭结果的基础格挡能力，并让格挡与现有公共动作语义保持一致，而不是另起一套平行规则。

## What I already know

* 当前代码层已经预留了 `Guard` 的动作类型、输入枚举和 `State.Action.Guard` Tag，但这些都还不是正式功能。
* 角色输入转发目前只看到 `NormalAttack` 与 `Dodge` 进入正式战斗输入评估。
* 还没有 `Ability.Guard`、Guard Ability、Guard 状态窗口或正式格挡结果改写逻辑。
* 父 task 已明确判断：
  如果没有“敌方攻击来源”和“玩家受击结果入口”，就不建议直接进入 Guard 正式实现。
* 因此，本 task 的正确定位不是“从零发明格挡阶段”，而是“把已经准备好的输入、动作语义、命中结果正式串起来”。

## Requirements

* 本 task 必须把 Guard 作为正式战斗能力接入现有结构，而不是临时写在 Character 私有状态机里。
* 至少需要完成以下正式接入：
  Guard 输入绑定；
  Guard Ability 或等价正式承载；
  Guard 动作状态；
  最小格挡判定窗口；
  来袭结果改写；
  调试可观察性。
* Guard 的承载应沿用当前第二章稳定路线：
  `UE5 + GAS + C++`；
  角色只负责输入转发、资源配置和调试入口；
  Guard 逻辑主体不应回退到旧式 Character 内部临时状态机。
* Guard 至少需要支持一个当前阶段可验收的最小交互：
  玩家在允许时机按下 Guard；
  进入 Guard 动作或 Guard 状态；
  在正确窗口内面对来袭时能把原本“命中玩家”的结果改写为“格挡成功”；
  不在窗口内或条件不满足时维持原始命中结果。
* 本轮允许 Guard 是“基础格挡”，不要求立刻区分完美格挡、反击派生、弹反、多方向盾牌角度或耐力条。
* Guard 与现有公共动作语义必须对齐：
  应纳入动作上下文；
  应有清晰阶段；
  应有明确开始、逻辑结束、真实结束原因；
  后续动作输入不应绕过公共语义层直接私下处理。
* Guard 与其他当前基础动作的关系必须明确：
  是否允许在空闲态直接进入；
  是否允许打断普通攻击 Recovery；
  是否允许与 Dodge 互斥；
  不允许靠“写死能用就行”来掩盖规则空洞。
* 允许当前阶段先采用单一 Guard 窗口、单一 Guard 表现、最小方向要求或甚至先不做方向要求，但必须在任务结论里明确哪些是当前阶段故意简化，而不是忘了做。
* 若程序发现当前最小受击结果闭环还不足以支撑 Guard 改写，应先回补前置 task，不要在本 task 内偷偷复制一套命中结果逻辑。

## Acceptance Criteria

* [ ] 玩家侧存在正式的 Guard 输入与 Guard 承载，不再只是枚举和 Tag 预留。
* [ ] Guard 可以在当前设计允许的时机进入，并被明确观察到开始、窗口与结束。
* [ ] 来袭攻击在 Guard 正确窗口内会产生“格挡成功”结果，而不是普通命中。
* [ ] 来袭攻击在 Guard 窗口外或条件不满足时，仍保持普通命中或既定失败结果。
* [ ] Guard 没有绕开当前公共动作语义层自创平行规则，能与现有动作上下文一致工作。
* [ ] 本 task 没有越界扩散到完美格挡、反击系统、耐力条、完整防御体系或大规模受击系统重做。

## Out of Scope

* 完美格挡 / Parry / 反击派生
* 耐力条、护盾值、防御属性和完整数值体系
* 多方向精细格挡角度系统
* 完整盾牌装备系统或防御装备体系
* 格挡后的所有正式特效、镜头、音效和重表现打磨

## Technical Notes

* 父 task：`05-19-chapter2-basic-combat`
* 前置依赖：
  `05-25-minimal-hostile-attack-probe`
  `05-25-player-hit-damage-minimum-loop`
  这两个任务至少要先提供：
  最小可复现来袭；
  玩家侧最小命中结果入口。
* 后置依赖：
  `05-25-guard-feedback-and-whitebox-validation` 应在本 task 的 Guard 逻辑已跑通后继续做表现与验收。
* 当前建议程序优先关注的代码现状：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h`
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.cpp`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Input/TwoHeartsAbilityInputID.h`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h`
  `Source/twohearts/twoheartsCharacter.cpp`
* 当前建议程序重点补齐的问题：
  `Guard` 输入如何进入 `HandleAbilityInputPressed` 与动作评估链；
  是否需要 `Ability.Guard` 新 Tag；
  Guard 的动作阶段、窗口和结束原因如何写入动作上下文；
  Guard 成功时如何改写上一任务提供的“玩家受击结果入口”。
* 若程序开工时仍缺信息，应优先回问：
  Guard 是按下生效、长按维持，还是先只做单次短窗口；
  本轮是否要求 Guard 能打断普通攻击 Recovery；
  本轮是否要求 Guard 对所有来袭都生效，还是先只支持最小敌对攻击探针。
* 当前允许的过渡实现边界：
  允许先只支持单一敌方攻击样本；
  允许先不做完美格挡和方向判定；
  允许先用简化动作资源或调试状态表示 Guard；
  但不允许把 Guard 主体逻辑写回 Character 私有状态机，或绕过玩家受击结果闭环直接写死“成功/失败”。
* 对应上游判断来源：
  `05-19-chapter2-basic-combat/info.md` 中的“2026-05-25 格挡阶段入口评估”。

