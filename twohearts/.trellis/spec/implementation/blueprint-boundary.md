# 蓝图与 C++ 分工边界

## C++ 负责

1. 核心逻辑
2. 状态写入与清理
3. 输入评估
4. 打断关系
5. 冷却与限制状态
6. 调试日志和联调口径

## 蓝图/数据负责

1. 动画资源
2. Montage Notify 时机
3. 表现层参数
4. 特效、镜头、位移曲线等表现配置

## 分工原则

1. 最终状态口径以 `C++` 为准
2. 蓝图驱动表现，但不擅自成为核心规则真相源
3. 若一个能力的正式位移主路径已确定为 `Root Motion`，不要再用硬推位置替代正式方案

## 普攻 Montage / Notify 联调约定

1. 若 `C++` 监听的是 `OnPlayMontageNotifyBegin` 一类 Montage Notify 回调，资源侧必须放真正的 `Montage Notify`，不要把通过 `Add Notify -> New Notify...` 新建的普通动画 notify 当成等价替代。
2. 需要用于切段、阶段推进或动作逻辑切换的关键标记，优先明确为 `Montage Notify`；若要求时机更稳定，可继续检查是否应设为 `Branching Point`。
3. 当联调现象表现为“程序看起来没吃到 notify”时，先验证运行时日志里是否真的出现对应的 `MontageNotifyBegin` / `MontageNotify`，不要先靠肉眼反复挪 notify 时间点。
4. 当怀疑段资源播错时，先用日志确认运行时实际命中的 Montage / Section / Sequence，再判断是资源串段、notify 时机错误，还是 Montage 混合把起手表现吞掉。
5. 若日志已经证明底层 Section 与 Sequence 命中正确，但体感仍像“下一段不明显”或“第二段像第三段”，优先复查 Montage 的 `Blend In / Blend Out` 等混合参数，不要直接把问题归因到 `C++` 选段错误。
6. 需要长期依赖的切段、阶段推进与打断判断，`C++` 仍应保留 fallback 或日志探针；但一旦资源联调完成，最终手感应优先靠正确的 Montage Notify 类型与时机收口。

## 来源

由旧文档 `../docs/程序技术文档/a基础闪避正式落地技术文档.md`、`../docs/程序技术文档/a基础战斗公共语义层技术文档.md` 提炼。
