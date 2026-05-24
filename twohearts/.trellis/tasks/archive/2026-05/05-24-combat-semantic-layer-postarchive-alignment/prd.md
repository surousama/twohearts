# 公共战斗语义层收尾后的现状对齐与进度回写

## Goal

作为 `05-21-combat-semantic-layer-readiness` 归档后的主程序收口 task，通读该阶段落下来的代码与文档，判断当前公共战斗语义层、预输入准备度、格挡前置条件和基础战斗模块补齐顺序，并把稳定结论回写到项目总览进度文档。

## What I already know

* `05-21-combat-semantic-layer-readiness` 已归档完成
* 当前用户希望主程序直接对齐现状，而不是继续新拆 review 类只读 task
* 当前第二章主线顺序口径仍是：基础闪避正式落地 -> 公共战斗语义层 -> 最小预输入 -> 受击与伤害 -> 格挡
* 本 task 需要回写的目标文档是 `../docs/a双心印开发进度简介.md`

## Requirements

* 通读公共战斗语义层相关代码，判断当前“最小预输入”是否已经真正完成，还是只具备接入口/前置条件
* 判断当前是否已具备进入格挡开发的前置条件，并说明仍缺什么
* 结合当前整体项目基础，给出武器、伤害系统、敌人系统、角色手感等内容的建议补齐时机
* 将当前稳定结论与阶段进度增量整合回 `../docs/a双心印开发进度简介.md`
* 输出结论需以主程序视角聚焦：结构 readiness、阶段顺序、缺口与建议，而不是展开成实现细节长报告

## Acceptance Criteria

* [ ] 明确回答当前是否已有完善预输入系统
* [ ] 明确回答当前是否具备格挡开发条件
* [ ] 明确给出基础内容补齐的建议时机与顺序
* [ ] `../docs/a双心印开发进度简介.md` 已回写当前稳定进度与现状口径

## Out of Scope

* 继续新增战斗业务代码
* 新建与本轮结论无关的 review task
* 直接展开格挡、伤害或敌人系统实现

## Technical Notes

* 重点代码落点预计包括：
  `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
  `Source/twohearts/twoheartsCharacter.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
  `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
* 重点文档来源包括：
  `../docs/a双心印开发进度简介.md`
  `.trellis/spec/project/current-progress.md`
  `.trellis/spec/combat/current-stage.md`
