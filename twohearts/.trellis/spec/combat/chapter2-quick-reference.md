# 第二章基础战斗高频入口速查

## 这份速查怎么用

1. 开工前先读这页，再决定是否展开看代码地图或源码。
2. 主程序用它快速判断“这次需求落在哪一层、会不会改顺序”。
3. 资深程序用它快速定位“该先打开哪几个文件”。

## 当前阶段一句话

第二章当前已经完成玩家防守侧最小正式闭环，下一步重点是补齐“玩家打敌方”的正式闭环、玩家受击正式表现、完整 Guard 表现和更稳定的战斗状态真相源。

## 先读哪份文档

1. 阶段判断：`combat/current-stage.md`
2. 代码全景：`combat/chapter2-code-map.md`
3. 这页：只负责 30 秒内快速定位

## 高频代码入口

1. 动作上下文与 phase：
   `Source/twohearts/TwoHearts/Combat/TwoHeartsCombatActionContextComponent.*`
2. 攻击描述与 Guard 规则字段：
   `Source/twohearts/TwoHearts/Combat/TwoHeartsAttackMetadata.h`
3. 普通攻击 Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.*`
4. Dodge Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.*`
5. Guard Ability：
   `Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.*`
6. 玩家输入、预输入、配置桥接：
   `Source/twohearts/twoheartsCharacter.*`
7. 玩家受击 / 伤害 / GuardOutcome：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.*`
8. 最小敌对攻击样本：
   `Source/twohearts/TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.*`
9. 调试 HUD：
   `Source/twohearts/twoheartsDebugHUD.cpp`

## 常见任务先看哪里

1. 拆 task / 判顺序：
   `current-stage.md` + `chapter2-code-map.md`
2. 普攻连段、命中、预输入：
   `TwoHeartsGA_NormalAttackBase.*` + `twoheartsCharacter.*`
3. Guard 规则、Guard 改写、Guard 结算：
   `TwoHeartsGA_Guard.*` + `TwoHeartsHostileAttackReceiverComponent.*`
4. 玩家受击、伤害、恢复：
   `TwoHeartsHostileAttackReceiverComponent.*`
5. probe 样本、白盒攻防入口：
   `TwoHeartsHostileAttackProbeCharacter.*`
6. 只想看运行态发生了什么：
   `twoheartsDebugHUD.cpp`

## 当前真相源

1. 动作状态真相源：
   `UTwoHeartsCombatActionContextComponent`
2. 攻击语义真相源：
   `FTwoHeartsAttackMetadata`
3. 玩家侧命中 / 伤害 / Guard / 受击真相源：
   `UTwoHeartsHostileAttackReceiverComponent`
4. 输入缓存与评估真相源：
   `AtwoheartsCharacter` + `CombatActionContext`

## 当前不要误判的点

1. Guard 已经不是“还没开始”的阶段。
2. 预输入二期已经存在，不要再按最小预输入阶段读代码。
3. 玩家受击状态已经存在，但正式表现层还没收完。
4. 普攻已经有攻击元数据模板，但还没正式接成“我打敌”的消费闭环。
