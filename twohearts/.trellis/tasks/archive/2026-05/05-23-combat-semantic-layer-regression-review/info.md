# 公共战斗语义层收口复核与回归拆单 - 主程序结论

## 结论摘要

1. `05-21-combat-semantic-layer-readiness` 的首轮实施目标，从代码覆盖角度看已经基本完成，不存在“还差一块核心能力完全没落代码”的阻塞缺口。
2. 四个实施子 task 中，前 3 个已经完成并归档；第 4 个 `combat-input-evaluation-preinput-hook` 的目标也已经体现在当前工作区代码中：
   `UTwoHeartsCombatActionContextComponent::EvaluateInputForAction`
   `AtwoheartsCharacter::HandleAbilityInputPressed`
   `ATwoheartsDebugHUD::DrawHUD`
3. 因此，父 task 当前更准确的状态不是“继续补一大块新代码”，而是：
   代码层首轮能力已到位；
   仍需做收口评估、回归验证与后续重构拆单。

## 为什么判断为“首轮代码已基本完成”

1. 公共动作上下文底座已经正式存在，并承载：
   `ActionType`
   `ActionPhase`
   `ActionEndReason`
   `LogicEnded`
2. 普攻已经不再只在私有实现里维护阶段语义，而是同步进入公共动作上下文。
3. Dodge 已经桥接到同一套上下文，并通过统一入口判断是否允许打断当前普攻。
4. 输入评估层已经形成正式输出口径：
   `ExecuteNow`
   `BufferInput`
   `Reject`
5. HUD 与结构化日志也已经补上最小观察口径，具备白盒回归基础。

## 当前仍需收口的内容

### 1. 不是“缺功能”，而是“缺验证”

当前还没有证据表明这一轮已经完成：

1. 编译验证
2. Unreal Editor 内 PIE 跑测
3. 白盒问题回归

所以当前状态应视为：
代码首轮完成，验证未完成。

### 2. 存在适合立即拆出的重构/硬化点

当前 `AtwoheartsCharacter::HandleAbilityInputPressed` 同时承担了：

1. 输入类型映射
2. 公共输入评估调用
3. active ability 转发
4. ability 扫描与激活
5. debug 事件记录

这让“统一输入评估”虽然已经出现，但消费边界仍偏重地压在 Character 里。  
一旦后续继续接“最小预输入”或新增动作类型，这里会快速膨胀。

同时，`UTwoHeartsCombatActionContextComponent::EvaluateInputForAction` 内已经开始承载带有普通攻击私有特征的缓冲判定。  
这在当前阶段是可接受的首轮实现，但不适合作为下一轮继续叠加的长期结构。

## 主程序判断

1. 对父 task 本身：
   当前不要求继续补一块新的核心业务代码，父 task 已具备进入“总体回归与收口”阶段的条件。
2. 对当前代码质量：
   建议立即补一个资深程序任务，目标不是推翻现有实现，而是把“统一输入评估”和“后续最小预输入承接点”之间的边界收紧。
3. 对验收顺序：
   应先做白盒回归与 PIE 跑测；
   若测试暴露边界问题，再由资深程序任务承接修正；
   若测试整体稳定，则父 task 可进入最终收口与归档准备。

## 本轮新拆出的后续 task

1. `05-23-combat-input-evaluation-polish`
   用于资深程序对当前输入评估与消费边界做收口整理和轻量重构。
2. `05-23-combat-semantic-layer-test-pass`
   用于资深测试工程梳理白盒回归点，并同步给策划一份 PIE 跑测关注点。

## 当前建议状态口径

对 `05-21-combat-semantic-layer-readiness` 建议表述为：

1. 首轮代码目标已完成
2. 尚未完成总体回归
3. 仍需测试验证与必要的边界收口
