# 格挡反馈与白盒验收进度记录

## 2026-05-27 当前状态

1. 当前 task 已进入执行阶段，主目标是把 Guard 从“逻辑已接通”推进到“可观察、可判断、可复核”。
2. 本 task 以资深测试工程师视角推进，不再扩写业务代码，重点补齐测试观察入口、白盒验收记录和剩余问题归因。

## 当前最小可读反馈包

1. 已有结构化日志，可观察：
   `AttackStarted`、`HitWindowOpened`、`AttackContact`、`HitConfirmed`、`GuardRewrite`、`GuardRewriteSuccess`、`GuardWindowOpened`、`GuardWindowClosed`、`GuardFinished`。
2. 已有调试 HUD / Overlay，可观察：
   Guard 是否激活；
   Guard 窗口是否开启；
   当前 Guard phase；
   最近一次 Guard 事件；
   最近一次玩家受击结果是否为 `GuardRewritten`。
3. 已有本地调试日志文件：
   `Saved/CombatDebug/normal-attack-debug.log`
   `Saved/Logs/twohearts.log`

## 当前口径下的最终配置

1. 当前代码中的基础 Guard 配置为：
   `Startup = 0.00s`
   `Window = 0.45s`
   `Recovery = 0.10s`
2. 为证明 Guard 改写链路，2026-05-27 跑测中曾临时把窗口放大到 `1.00s`，并用该配置拿到连续成功样本。
3. 在确认改写链路稳定后，窗口已按当前要求收回到 `0.45s`。

## 本轮已明确验证的内容

1. 空闲态可直接进入 Guard。
2. Guard 成功时，玩家受击结果会从 `HitConfirmed` 改写为 `GuardRewritten`。
3. Guard 成功与普通受击在日志和 HUD 中已可区分，不再只能靠体感猜测。
4. 过早输入时，Guard 窗口会先关闭，随后敌对命中落成 `HitConfirmed`，不会误触发 `GuardRewriteSuccess`。
5. 过晚输入时，敌对命中会先落成 `HitConfirmed`，随后才出现 `GuardActivate`，不会再回写为 `GuardRewritten`。
6. 完全不输入 Guard 时，敌对命中会按预期直接落成 `HitConfirmed`。
7. 连续重复测试下，Guard 改写链路已证明具备稳定一致性。
8. Guard 可打断普通攻击 `Recovery / LogicEnded`。
9. Guard 不可在普通攻击 `Startup / Active` 越权打断。
10. Guard 当前只允许从无动作态、普通攻击 `Recovery / LogicEnded` 或当前基础 Dodge 进入。

## 结论

1. 当前 Guard 的基础逻辑链路已基本达标。
2. 当前最小反馈口径与白盒验收口径已补齐，可进入归档收尾。
3. 本轮已判断：没有新增需要写回 `.trellis/spec/` 的稳定项目级规范，当前结论保留在 task 验收文档内即可。
