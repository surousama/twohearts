# 公共战斗语义层收口复核与回归拆单

## Goal

作为 `05-21-combat-semantic-layer-readiness` 的主程序收口子 task，复核该父 task 及其 4 个实施子 task 的真实完成状态，判断当前是否还存在必须继续落代码的缺口，并在必要时继续拆出资深程序整改任务与专项测试任务。

## What I already know

* 父 task `05-21-combat-semantic-layer-readiness` 当前状态为 `in_progress`，并登记了 4 个子 task
* 其中 4 个子 task 目录均已存在或已归档，前三个对应代码提交分别为：
  `cf029c8`
  `5024f1a`
  `eb0a444`
* 当前工作区还存在一组未提交改动，主要落在：
  `TwoHeartsCombatActionContextComponent.*`
  `twoheartsCharacter.*`
  `twoheartsDebugHUD.cpp`
* 从代码检索结果看，当前已经存在统一输入评估入口 `EvaluateInputForAction`，并输出：
  `ExecuteNow / BufferInput / Reject`
* 本轮角色为主程序，优先产出状态判断、风险评估、回归建议与派工拆分，而不是直接继续写业务代码

## Requirements

* 复核父 task 原始目标与 4 个子 task 的产物，判断其是否已经覆盖父 task 的首轮实施边界
* 明确给出“是否还需要继续落代码”的主程序结论，并区分：
  必须现在补
  可以后续独立整改
  当前可接受已知限制
* 对当前代码改动做初步评估，指出结构风险、回归风险、未收口点和文档/归档缺口
* 如果评估后认为需要修改、优化或重构，必须新建一个面向资深程序的子 task，并写清目标、边界、验收口径
* 新建一个面向测试的子 task，覆盖：
  白盒测试工程需要关注的点
  策划在游戏内 PIE 跑测需要关注的点
* 输出结论必须可直接服务父 task 收口与下一轮推进，不能只停留在聊天结论

## Acceptance Criteria

* [ ] 已明确父 task 当前完成状态与是否仍需继续落代码
* [ ] 已对现有代码给出主程序视角的初步评估与风险判断
* [ ] 若需要资深程序继续整改，已创建新 task 并写好可直接执行的 `prd.md`
* [ ] 已创建新的测试 task，并写清白盒测试与 PIE 跑测关注点
* [ ] 当前结论已沉淀到 Trellis task 文档中，可供后续继续流转

## Out of Scope

* 本轮直接修改战斗业务代码
* 本轮直接执行完整 Unreal Editor PIE 跑测
* 本轮直接代替资深测试写最终测试报告
* 最小预输入完整功能实现

## Technical Notes

* 父 task：`05-21-combat-semantic-layer-readiness`
* 主要上游子 task：
  `05-23-combat-action-context-foundation`
  `05-23-normal-attack-semantic-bridge`
  `05-23-dodge-semantic-bridge-and-interrupt-unification`
  `05-23-combat-input-evaluation-preinput-hook`
* 当前重点代码落点：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/twoheartsDebugHUD.cpp`
