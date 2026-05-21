# 文档用途

1. 这份文档用于记录普通攻击最小闭环测试阶段新增的调试能力。
2. 本文档只描述测试辅助功能，不描述业务玩法逻辑。

# 调试功能概览

1. 结构化日志：在普通攻击关键节点输出统一格式日志，便于定位输入、播段、推进和重置过程。
2. 游戏内调试面板：在游戏画面中实时显示当前普攻状态、最近失败原因和最近事件列表。
3. 调试开关：通过控制台命令控制面板显示、日志输出和详细日志开关。
4. 最近事件列表：在角色实例内缓存最近若干条普攻事件，便于回看状态流转。

# 结构化日志

1. 日志类别：`LogtwoheartsCombatTest`
2. 日志用途：记录普通攻击测试过程中的关键状态变化和失败原因。
3. 覆盖节点：输入触发、请求起手、缓存下一段、播放段落、段落结束、连段重置、失败分支。

# 游戏内调试面板

1. 显示位置：默认显示在游戏画面左上角。
2. 显示内容：当前是否攻击中、当前段序、是否已缓存下一段、最近失败原因、最近事件列表。
3. 使用目的：不打开日志窗口时，也能直接观察普通攻击内部状态。

# 调试开关

1. `NormalAttackDebugPanel 0/1`：关闭或开启普通攻击调试面板。
2. `NormalAttackDebugLog 0/1`：关闭或开启普通攻击结构化日志。
3. `NormalAttackDebugVerbose 0/1`：关闭或开启普通攻击详细日志。
4. `NormalAttackDebugClear`：清空最近事件列表和最近失败原因。

# 最近事件列表

1. 默认数量：默认保留最近 `12` 条事件。
2. 记录内容：时间、事件名、段序、是否攻击中、是否缓存下一段、Section 名称、事件说明。
3. 扩展方式：可通过角色上的 `NormalAttackDebugMaxEvents` 调整保留数量。

# 代码落点

1. 角色调试数据与日志记录：`twohearts/Source/twohearts/twoheartsCharacter.h/.cpp`
2. 调试面板绘制：`twohearts/Source/twohearts/twoheartsDebugHUD.h/.cpp`
3. 控制台调试命令：`twohearts/Source/twohearts/twoheartsPlayerController.h/.cpp`
4. HUD 挂接入口：`twohearts/Source/twohearts/twoheartsGameMode.cpp`

# 使用建议

1. 冒烟测试时建议同时开启调试面板和结构化日志。
2. 复现边界问题时建议保留详细日志开启状态。
3. 准备新一轮测试前建议先执行一次 `NormalAttackDebugClear`。
