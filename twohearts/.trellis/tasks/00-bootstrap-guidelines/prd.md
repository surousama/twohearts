# Bootstrap 任务：补齐项目开发规范

**你（AI）正在执行这个任务。开发者本人不会直接阅读这个文件。**

开发者刚刚第一次在这个项目中运行了 `trellis init`。
现在 `.trellis/` 已经生成，但其中的 spec 还只是空的脚手架；
这个 bootstrap task 也已经放在 `.trellis/tasks/` 下。
当他们准备处理这件事时，应在一个能提供 Trellis session identity 的会话中启动这个任务。

**你的工作**：帮助他们把团队真实的编码约定补进 `.trellis/spec/`。
未来这个项目中的每次 AI 会话，都会让 `trellis-implement` 与 `trellis-check`
子代理自动读取 per-task jsonl manifest 中列出的 spec 文件。
spec 为空，子代理就会写出通用模板式代码；
spec 真实，子代理才会贴合团队实际模式。

不要一上来倾倒长篇说明。先用一句简短开场，判断仓库里是否已有约定文档
（例如 `CLAUDE.md`、`.cursorrules` 等），再以对话方式继续推进。

---

## 当前状态（每完成一项就更新复选框）

- [ ] 补齐后端规范
- [ ] 补齐前端规范
- [ ] 增加代码示例

---

## 需要补充的 Spec 文件

### 后端规范

| 文件 | 应记录什么 |
|------|------------------|
| `.trellis/spec/backend/directory-structure.md` | 各类文件应放在哪里（routes、services、utils 等） |
| `.trellis/spec/backend/database-guidelines.md` | ORM、migrations、查询模式、命名约定 |
| `.trellis/spec/backend/error-handling.md` | 错误如何捕获、记录与返回 |
| `.trellis/spec/backend/logging-guidelines.md` | 日志等级、格式、该记录什么 |
| `.trellis/spec/backend/quality-guidelines.md` | 代码评审标准、测试要求 |

### 前端规范

| 文件 | 应记录什么 |
|------|------------------|
| `.trellis/spec/frontend/directory-structure.md` | component / page / hook 的组织方式 |
| `.trellis/spec/frontend/component-guidelines.md` | 组件模式、props 约定 |
| `.trellis/spec/frontend/hook-guidelines.md` | 自定义 hook 的命名与模式 |
| `.trellis/spec/frontend/state-management.md` | 状态库、常用模式、不同状态放在哪里 |
| `.trellis/spec/frontend/type-safety.md` | TypeScript 约定、类型组织方式 |
| `.trellis/spec/frontend/quality-guidelines.md` | lint、测试、可访问性 |

### 思考指南（已预填）

`.trellis/spec/guides/` 中已经放入通用思考指南与最佳实践。
只有在明显不适合本项目时，才需要定制修改。

---

## 如何补齐 spec

### 第一步：优先从已有约定文档导入（推荐）

先搜索仓库中现有的约定文档。若存在，就先阅读它们，
再把相关规则提炼到对应的 `.trellis/spec/` 文件里。
这通常比从零撰写快得多。

| 文件 / 目录 | 对应工具 |
|------|------|
| `CLAUDE.md` / `CLAUDE.local.md` | Claude Code |
| `AGENTS.md` | Codex / Claude Code / 兼容 agent 的工具 |
| `.cursorrules` | Cursor |
| `.cursor/rules/*.mdc` | Cursor（rules 目录） |
| `.windsurfrules` | Windsurf |
| `.clinerules` | Cline |
| `.roomodes` | Roo Code |
| `.github/copilot-instructions.md` | GitHub Copilot |
| `.vscode/settings.json` → `github.copilot.chat.codeGeneration.instructions` | VS Code Copilot |
| `CONVENTIONS.md` / `.aider.conf.yml` | aider |
| `CONTRIBUTING.md` | 通用项目约定 |
| `.editorconfig` | 编辑器格式规则 |

### 第二步：对未被现有文档覆盖的部分做代码库分析

从真实代码中识别模式。写每个 spec 文件之前：
- 在代码库中找到 2 到 3 个真实例子
- 引用真实文件路径，而不是假设路径
- 记录团队明确在避免的反模式

### 第三步：记录现实，而不是理想

**关键点**：写代码库**现在实际怎么做**，不是你认为它未来应该怎么做。
子代理会按 spec 行事；如果 spec 里写的是代码库里根本不存在的理想模式，
子代理写出的代码就会显得格格不入。

如果团队已有明确技术债，就如实记录当前状态；
“如何改进”是另一场对话，不属于 bootstrap 阶段。

---

## 运行时原理速讲

当开发者问“为什么一定要写 spec”时，可以这样解释：

- 每个 AI 编码任务都会派生两个子代理：`trellis-implement`（负责写代码）与 `trellis-check`（负责验质量）
- 每个 task 都有 `implement.jsonl` / `check.jsonl` 清单，用来列出要加载哪些 spec 文件
- 平台 hook 会把这些 spec 文件以及该 task 的 `prd.md` 自动注入每个子代理的 prompt
- 因此，子代理会按团队约定编码/评审，而不需要每次手动粘贴规则
- 真相源就是 `.trellis/spec/`；这也是为什么现在把它填好，后面会一直受益

---

## 完成条件

当开发者确认上面的勾选项都已用真实示例填好，而不是占位文本后，引导他们运行：

```bash
python ./.trellis/scripts/task.py finish
python ./.trellis/scripts/task.py archive 00-bootstrap-guidelines
```

归档完成后，后续新加入项目的开发者拿到的将是
`00-join-<slug>` onboarding task，而不再是这个 bootstrap task。

---

## 建议开场白

“欢迎来到 Trellis。刚才的 init 已经把我准备好了，我可以帮你把项目 spec 补齐。
这是一轮一次性的配置工作，做完后，未来每次 AI 会话都会按团队约定行事，
而不是继续写通用模板代码。开始之前，你这边有现成的约定文档
（比如 `CLAUDE.md`、`.cursorrules`、`CONTRIBUTING.md`）可供提取吗？
如果没有，我就从代码库本身开始扫描。”
