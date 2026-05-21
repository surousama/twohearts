# 跨层思考指南

> **用途**：在实施前先想清楚数据是如何穿过各层流动的。

---

## 问题本质

**大多数 bug 都发生在层与层的边界，而不是层内部。**

常见的跨层 bug 包括：
- API 返回的是格式 A，前端却按格式 B 解析
- 数据库存的是 X，服务层转成 Y 时丢了信息
- 多个 layer 以不同方式重复实现了同一逻辑

---

## 实施跨层功能之前

### 第一步：画出数据流

先把数据如何移动梳理出来：

```
来源 → 转换 → 存储 → 读取 → 再转换 → 展示
```

对每一段箭头都问自己：
- 这里的数据格式是什么？
- 这里可能出什么问题？
- 谁负责校验？

### 第二步：识别边界

| 边界 | 常见问题 |
|----------|---------------|
| API ↔ Service | 类型不匹配、字段缺失 |
| Service ↔ Database | 格式转换、null 处理 |
| Backend ↔ Frontend | 序列化、日期格式 |
| Component ↔ Component | props 结构变化 |

### 第三步：定义契约

对每一个边界都明确：
- 精确的输入格式是什么？
- 精确的输出格式是什么？
- 这里可能出现哪些错误？

---

## 常见跨层错误

### 错误 1：默认假设格式一致

**坏例子**：不检查就假设日期格式一致

**好例子**：在边界处显式做格式转换

### 错误 2：校验散落各处

**坏例子**：在多个 layer 重复校验同一件事

**好例子**：在入口层集中校验一次

### 错误 3：抽象泄漏

**坏例子**：组件直接知道数据库 schema

**好例子**：每一层只了解它的相邻层

---

## 跨层功能检查清单

实施前：
- [ ] 已梳理完整数据流
- [ ] 已识别所有层边界
- [ ] 已定义每个边界的数据格式
- [ ] 已决定校验发生在哪一层

实施后：
- [ ] 已用边界条件测试（null、空值、非法值）
- [ ] 已核对每一层的错误处理
- [ ] 已确认数据往返过程不会丢失

---

## 跨平台模板一致性

在 Trellis 中，命令模板（例如 `record-session.md`）通常会在**多个平台**上同时存在，内容完全一致或高度相近。这本身就是一种跨层边界。

### 修改任意命令模板后的检查清单

- [ ] 找出拥有同名命令的所有平台：`find src/templates/*/commands/trellis/ -name "<command>.*"`
- [ ] 同步更新所有平台副本（Markdown `.md` 与 TOML `.toml`）
- [ ] 若涉及 Gemini TOML，注意调整续行写法（`\\` vs `\`）与三引号字符串
- [ ] 运行 `/trellis:check-cross-layer`，确认没有漏改

**真实案例**：在 Claude 版本中把 `record-session.md` 改为使用 `--mode record`，却漏掉了 iFlow、Kilo、OpenCode 和 Gemini，最终由 cross-layer 检查抓出来。

---

## 生成型运行时模板的升级一致性

有些生成出来的文件既是文档，也是运行时输入。在 Trellis 里，`.trellis/workflow.md` 会被 `get_context.py`、`workflow_phase.py`、SessionStart 过滤器，以及每轮 hook 一起解析。凡是动到这种模板的改动，都必须同时验证 fresh init 和 upgrade path。

### 修改“会被运行时解析的模板”后的检查清单

- [ ] 找出所有读取该模板的运行时解析器，而不只是写入该文件的安装逻辑
- [ ] 检查关键语法是否分布在明显的 managed region 之外，例如 tag blocks 之外的文本
- [ ] 同时验证 fresh `init` 输出，以及一个会写入旧版 `.trellis/.version` 的 `update` 升级场景
- [ ] 增加一条 upgrade 回归测试：用旧版 pristine template fixture 升级后，断言安装结果达到当前模板形状
- [ ] 更新拥有该运行时契约的 backend spec

**真实案例**：Codex inline 模式把 workflow 平台标记从 `[Codex]` / `[Kilo, Antigravity, Windsurf]` 改为 `[codex-sub-agent]` / `[codex-inline, Kilo, Antigravity, Windsurf]`。fresh init 没问题，但 `trellis update` 只合并了 `[workflow-state:*]` 块，保留了这些块外面的旧标记。结果是升级过的项目拿到了新的 hook 脚本，却保留了旧的 workflow 路由，导致 `get_context.py --mode phase --platform codex` 在 Phase 2.1 可能返回空结果。

---

## 模式探测检查清单

当某个 CLI 通过探测远程资源来自动判断模式（例如检查 `index.json` 是否存在，以决定走 marketplace 还是 direct download）时：

### 实施前
- [ ] 这个探测是否覆盖了**所有**会使用探测结果的代码路径（交互式、`-y`、`--flag` 组合等）
- [ ] 是否明确区分了 404 和瞬时错误，而不是都当成“未找到”
- [ ] 遇到瞬时错误时是否会**中止或重试**，而不是静默切到错误模式
- [ ] 当上下文变化（例如切换 source）时，共享状态（缓存、预取数据）是否会被**重置**
- [ ] **快捷路径**（例如 `--template` 跳过选择器）是否拥有与正常探测路径同等级别的错误处理；检查下游函数是否仍调用 catch-all wrapper

### 实施后
- [ ] 已追踪从探测结果到模式分支的所有路径，没有隐性 fallthrough
- [ ] 外部格式契约（giget URI、raw URL）已通过测试，或至少写成注释说明
- [ ] 元数据读取会消耗完整响应，或使用流式解析器；绝不把固定长度前缀当成完整 JSON 去解析
- [ ] 若需从多个字段重组复合标识符，已确认**所有字段都包含在内**，且**顺序正确**（例如 `provider:repo/path#ref`，而不是 `provider:repo#ref/path`）
- [ ] 已确认快捷路径后调用的**动作函数**不会在内部悄悄回退到旧的 catch-all fetch；在需要区分错误类型时，必须使用具备探测质量的变体

**真实案例**：自定义 registry 流程在 3 轮 review 中连续暴露了 8 个 bug：（1）探测只在交互模式下运行；（2）瞬时错误会落到错误模式；（3）giget URI 把 `#ref` 放错位置；（4）预取模板数据在切换 source 后没有清空；（5）`--template` 快捷路径跳过了 probe，但 `downloadTemplateById` 内部仍调用 catch-all `fetchTemplateIndex`，把 timeout 误报成 “Template not found”。

**真实案例**：agent-session 更新提示拉取 npm `latest` 元数据时只执行了 `response.read(4096)`，然后直接把结果当作完整 JSON 解析。`@mindfoldhq/trellis` 的包元数据超过 4 KB，导致 JSON 被截断、解析静默失败，首次会话注入时也看不到更新提示。修复方式：读取完整响应再解析，并补一条回归测试，模拟 `version` 后面还跟着 8 KB 元数据尾巴的情况。

---

## 什么时候需要专门写流程文档

在以下情况中，建议补一份详细 flow 文档：
- 功能横跨 3 层以上
- 需要多个团队协作
- 数据格式复杂
- 这个功能以前出过问题
