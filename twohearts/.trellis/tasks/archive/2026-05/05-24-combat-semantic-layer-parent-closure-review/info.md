# 公共战斗语义层父任务收尾判定 - 主程序结论

## 当前结论

当前**不建议**直接将 `05-21-combat-semantic-layer-readiness` 判定为“已经可以正式收尾归档”。

更准确的状态口径是：

1. 父 task 的首轮实施目标已经基本完成
2. 代码层核心子项已经落地并归档
3. 但总体收口链路还没有完全闭环，仍差最后一段回归验证与子 task 收尾

## 为什么说“首轮实施目标已基本完成”

父 task 原始验收口径主要是：

1. 给出“是否允许进入公共战斗语义层”的主程序结论
2. 若允许进入，拆出首轮可执行子 task 与实施顺序
3. 输出结果可作为后续资深程序实施依据

从现有 task 证据看，这一层目标已经被实际完成：

1. `05-23-combat-action-context-foundation` 已归档  
   对应最小公共动作上下文底座
2. `05-23-normal-attack-semantic-bridge` 已归档  
   对应普攻桥接到公共动作上下文
3. `05-23-dodge-semantic-bridge-and-interrupt-unification` 已归档  
   对应 Dodge 桥接与统一打断入口
4. `05-23-combat-input-evaluation-preinput-hook` 已归档  
   对应统一输入评估与最小预输入接入口

同时，`05-23-combat-semantic-layer-regression-review/info.md` 已明确写出：

1. 父 task 首轮代码目标已完成
2. 当前不再是“缺核心代码能力”，而是“缺总体回归与收口”

## 为什么现在还不能直接收尾

### 1. 仍有直接子 task 未收尾

`05-23-combat-semantic-layer-regression-review` 目前仍是 `in_progress`，还没有归档。  
对父 task 而言，这说明“收口复核”这条子链路本身尚未完成。

### 2. 现有测试结论仍保留运行态验证缺口

`archive/2026-05/05-23-combat-semantic-layer-test-pass/info.md` 已记录：

1. 本轮做了代码 review 与调试链路检查
2. 已完成本地构建验证
3. **本轮未做 Unreal Editor PIE 实跑**
4. 三个缺陷已做代码修复，但仍写明“待你继续用 PIE 复测确认”

这意味着当前证据足以说明“代码已经进入可验证状态”，但还不足以说明“公共战斗语义层已经完成总体回归，可无保留收口”。

### 3. 当前 active task 状态与文档结论尚未对齐

目前文档里的真实结论已经推进到：

1. 高优先级代码缺陷已定位并修复
2. 本地构建通过
3. 剩余主要风险集中在运行态回归确认

但 task 状态层仍停留在：

1. 父 task `05-21-combat-semantic-layer-readiness` 仍为 `in_progress`
2. 子 task `05-23-combat-semantic-layer-regression-review` 仍为 `in_progress`

也就是说，实际“收尾动作”还没走完。

## 主程序判断

如果问题是“这批首轮语义层代码是否已经做出来了”，答案是：**是，已经基本做完。**

如果问题是“`05-21-combat-semantic-layer-readiness` 这个父 task 现在能不能直接按已完成归档”，答案是：**现在还不建议。**

当前更合理的判定应为：

1. 父 task 已达到“等待最终回归确认”的阶段
2. 不是继续扩新功能
3. 而是先完成运行态回归结论，再收口归档

## 建议的收尾顺序

1. 先按 `05-23-combat-semantic-layer-test-pass/info.md` 的清单完成至少一轮 PIE 回归确认
2. 若 PIE 未复现阻塞问题，则补写 `05-23-combat-semantic-layer-regression-review` 的最终结论并归档该子 task
3. 在子 task 全部闭环后，再把 `05-21-combat-semantic-layer-readiness` 作为阶段父 task 收尾

## 若你想提前收尾，需要明确接受的风险

只有在你明确接受以下口径时，才适合提前结束父 task：

1. 当前将其视为“代码阶段完成”
2. PIE 运行态回归风险转移到下一轮 task 持续观察
3. 文档中明确保留“未完成最终运行态验证”的残余风险

如果不接受这三点，当前就不应直接收尾父 task。
