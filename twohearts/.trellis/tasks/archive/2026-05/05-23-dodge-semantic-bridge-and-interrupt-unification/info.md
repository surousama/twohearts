# 基础闪避语义桥接与打断统一 - 实现记录

## 本轮实现

1. 在 `UTwoHeartsCombatActionContextComponent` 增加 `CanCurrentActionBeInterruptedBy`，把“当前动作能否被某类动作打断”的首轮统一判定入口收口到公共层。
2. 让 `UTwoHeartsGA_Dodge` 正式桥接公共动作上下文：
   `Startup` 对应 Dodge 激活；
   `Active` 对应无敌帧开始；
   `Recovery` 对应无敌帧结束后的收尾阶段；
   `LogicEnded` 对应 `Dodge_Finished` 或实际结束前的可衔接时机；
   `FinishAction` 对应 Ability 真正结束。
3. 让 Dodge 激活时优先读取公共动作上下文决定是否允许打断当前动作；若当前动作是普攻且公共层允许，再调用普攻实例执行真实中断收尾。
4. 普攻侧新增 `TryInterruptByAction`，把“由谁打断”与“是否允许被打断”的职责拆开：
   允许性由公共层判定；
   普攻 Ability 只负责自身实际停招与收尾。
5. 调试 HUD 现在会显示公共动作上下文里的 `dodge_interrupt`，便于直接观察统一打断口径，而不是只看普攻私有调试字段。

## 当前边界

1. 本轮只把“普攻被 Dodge 打断”升级到公共入口，没有扩展成完整可配置打断规则系统。
2. 若公共动作上下文声明当前动作可被 Dodge 打断，但运行时找不到对应普攻实例，Dodge 会拒绝启动并记录明确调试事件；这属于当前首轮桥接的保守处理。
3. 输入评估与预输入仍留给后续 `05-23-combat-input-evaluation-preinput-hook`，本轮不提前实现。

## 验证

1. 已执行：
   `..\..\UE_5.6\Engine\Build\BatchFiles\Build.bat twoheartsEditor Win64 Development -Project="G:\twohearts\twohearts\twohearts.uproject" -WaitMutex -NoHotReloadFromIDE`
2. 结果：
   `twoheartsEditor` 构建成功。
