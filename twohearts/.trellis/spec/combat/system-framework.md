# 双心印战斗系统框架

## 一级模块

1. 基础战斗模块
2. 技能系统模块（结印系统）
3. 合印系统模块
4. 战斗数据与状态模块

## 基础战斗模块当前范围

1. 受攻击与受击反馈
2. 普通攻击
3. 预输入
4. 闪避
5. 格挡

## 长期共通语义

1. 动作阶段
2. 打断关系
3. 输入权限
4. 状态标签
5. 动作逻辑结束事件

## 公共动作上下文接入约定

1. 正式 Ability 动作接入公共语义层时，应优先写入 `UTwoHeartsCombatActionContextComponent`
2. 动作开始使用 `BeginAction`，并提供 `ActionType`、初始 `ActionPhase`、Ability Tag、动作状态 Tag 和可读实例名
3. 阶段切换使用 `TransitionToPhase`；进入 `LogicEnded` 时使用 `MarkLogicEnded`
4. Ability 收尾时使用 `FinishAction`，并明确写入 `Completed / Cancelled / Interrupted / Failed` 中的真实结束原因
5. 后续动作或输入评估应优先读取公共动作上下文，不应反查某个动作的私有调试字段作为长期真相源

## 对 AI 的要求

1. 任何子系统实现都不能自行发明一套与其他动作冲突的规则
2. 当前第二章是这些公共语义的上游需求来源
3. 如果局部功能多次复用相同概念，应推动收束公共语义层，而不是各写一套

## 来源

由旧文档 `../docs/a双心印战斗系统框架.md` 提炼。
