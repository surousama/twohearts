# 开发工作流

---

## 核心原则

1. **先判定角色，再开工** — 角色不清时，不进入实施
2. **先定边界，再定实现** — 当前目标、非目标、验收口径要先写清
3. **规范靠注入，不靠记忆** — 规范来自 `.trellis/spec/`，不依赖 AI 自己记住
4. **所有关键信息都要落盘** — 阶段结论、实现记录、测试反馈都应写入文件
5. **父 task 管阶段，子 task 管单轮验收** — 避免任务粒度失控
6. **迁移期允许回退** — 在新 workflow 跑通前，旧 `docs` 继续保留；跑通后 `docs/` 主要保留策划类文档维护

---

## Trellis 系统

### 开发者身份

首次使用时，先初始化你的身份：

```bash
python ./.trellis/scripts/init_developer.py <your-name>
```

这会创建 `.trellis/.developer`（已 gitignore）和 `.trellis/workspace/<your-name>/`。

### 规范系统

`.trellis/spec/` 存放按 package 与 layer 组织的编码规范。

- `.trellis/spec/<package>/<layer>/index.md`：入口索引，包含**开工前检查项**与**质量检查项**。具体规范在它链接出去的 `.md` 文件里。
- `.trellis/spec/guides/index.md`：跨 package 的通用思考指南。

```bash
python ./.trellis/scripts/get_context.py --mode packages   # 列出 package / layer
```

**何时更新 spec**：发现了新模式/新约定、需要把 bug 预防经验固化、形成了新的技术决策时。

### 任务系统

每个 task 在 `.trellis/tasks/{MM-DD-name}/` 下都有独立目录，包含 `prd.md`、`implement.jsonl`、`check.jsonl`、`task.json`，以及可选的 `research/`、`info.md`。

```bash
# 任务生命周期
python ./.trellis/scripts/task.py create "<title>" [--slug <name>] [--parent <dir>]
python ./.trellis/scripts/task.py start <name>          # 设置当前活跃 task（可用时按会话范围生效）
python ./.trellis/scripts/task.py current --source      # 显示当前活跃 task 及其来源
python ./.trellis/scripts/task.py finish                # 清除当前活跃 task（触发 after_finish hooks）
python ./.trellis/scripts/task.py archive <name>        # 移动到 archive/{year-month}/
python ./.trellis/scripts/task.py list [--mine] [--status <s>]
python ./.trellis/scripts/task.py list-archive

# 代码规范上下文（通过 JSONL 注入 implement/check agent）。
# `implement.jsonl` / `check.jsonl` 会在 `task create` 时为支持子代理的平台生成初始文件；
# AI 会在 Phase 1.3 中填入真正需要的 spec + research 条目。
python ./.trellis/scripts/task.py add-context <name> <action> <file> <reason>
python ./.trellis/scripts/task.py list-context <name> [action]
python ./.trellis/scripts/task.py validate <name>

# 任务元数据
python ./.trellis/scripts/task.py set-branch <name> <branch>
python ./.trellis/scripts/task.py set-base-branch <name> <branch>    # PR 目标分支
python ./.trellis/scripts/task.py set-scope <name> <scope>

# 层级关系（父/子任务）
python ./.trellis/scripts/task.py add-subtask <parent> <child>
python ./.trellis/scripts/task.py remove-subtask <parent> <child>

# 创建 PR
python ./.trellis/scripts/task.py create-pr [name] [--dry-run]
```

> 运行 `python ./.trellis/scripts/task.py --help` 获取权威、最新的命令列表。

**当前任务机制**：`task.py create` 会创建任务目录，并在会话身份可用时自动设置当前会话的活跃 task 指针，这样 planning 阶段的 breadcrumb 能立即生效。`task.py start` 会写入同一个指针（若已存在则幂等），并把 `task.json.status` 从 `planning` 改成 `in_progress`。状态保存在 `.trellis/.runtime/sessions/` 下。若 hook 输入、`TRELLIS_CONTEXT_ID` 或平台原生会话环境变量都没有提供上下文键，则不会有 active task，此时 `task.py start` 会报会话身份提示。`task.py finish` 会删除当前会话文件（task 状态不变）。`task.py archive <task>` 会先写入 `status=completed`，再移动目录到 `archive/`，并删除所有仍指向该已归档 task 的运行时会话文件。

### 工作区系统

用于在 `.trellis/workspace/<developer>/` 下记录每次 AI 会话，方便跨会话跟踪。

- `journal-N.md`：会话日志。**每个文件最多 2000 行**；超出后会自动新建 `journal-(N+1).md`
- `index.md`：个人索引（总会话数、最近活跃时间）

```bash
python ./.trellis/scripts/add_session.py --title "Title" --commit "hash" --summary "Summary"
```

### 上下文脚本

```bash
python ./.trellis/scripts/get_context.py                            # 完整会话运行时上下文
python ./.trellis/scripts/get_context.py --mode packages            # 可用的 package 与 spec layer
python ./.trellis/scripts/get_context.py --mode phase --step <X.Y>  # 某个工作流步骤的详细说明
```

---

<!--
  WORKFLOW-STATE BREADCRUMB CONTRACT（编辑下方标签块前请先阅读）

  `## Phase Index` 中嵌入的 4 个 [workflow-state:STATUS] 块，是每个支持平台
  在每轮对话中注入 `<workflow-state>` breadcrumb 的唯一真相源。
  inject-workflow-state.py（Python 平台）与
  inject-workflow-state.js（OpenCode 插件）只解析这些块；
  从 v0.5.0-rc.0 起，脚本内不再内置后备字典。

  STATUS 字符集要求：[A-Za-z0-9_-]。若 hook 找不到某个标签，
  会退化为通用提示 `"Refer to workflow.md for current step."`，
  这是刻意保留的显眼失败模式，方便用户尽快发现并修复损坏的 workflow.md。

  不变量（test/regression.test.ts）：
    每个被标记为 `[required · once]` 的工作流步骤，
    都必须在其所属阶段的 [workflow-state:*] 块中有对应的强化提示。
    breadcrumb 是每轮注入的唯一通道；如果某个必做步骤没在这里出现，
    AI 就会悄悄跳过它（Phase 1.3 的 jsonl curation 漏做、Phase 3.4 的 commit 漏做，
    都曾由这个缺口引发）。

  标签与阶段范围：
    [workflow-state:no_task]      → 没有活跃 task；位于 Phase 1 之前
    [workflow-state:planning]     → 覆盖整个 Phase 1（status='planning'）
    [workflow-state:in_progress]  → 覆盖 Phase 2 + Phase 3.1-3.4
                                    （状态会从 task.py start 一直保持为
                                    'in_progress'，直到 task.py archive）
    [workflow-state:completed]    → 当前实际上不会触发：cmd_archive 会在同一次调用里
                                    同时写状态并移动目录，解析器会先失去指针。
                                    该块保留，供未来做显式
                                    in_progress→completed 状态迁移时使用。

  编辑检查清单：
    - 修改某个 [workflow-state:STATUS] 块时，也要同步检查对应阶段中
      所有 `[required · once]` 步骤的文字是否仍一致
    - 编辑完成后运行 `trellis update`，把新内容推送到下游用户项目
      （这是按块替换的受管区域）
    - 完整运行时契约见：
      .trellis/spec/cli/backend/workflow-state-contract.md
-->

## Phase Index

```
Phase 1: Plan    → 搞清楚要做什么（brainstorm + research → prd.md）
Phase 2: Execute → 实施改动并通过质量检查
Phase 3: Finish  → 沉淀经验并收尾
```

<!-- Per-turn breadcrumb: shown when there is no active task (before Phase 1) -->

[workflow-state:no_task]
当前没有活跃 task。**A 直接答复** — 纯问答、阅读说明、流程解释、工具介绍、轻量查询，可直接回答。
**B 创建 task** — 任何会落文档、改配置、改代码、做测试计划、做技术评估、推进真实开发阶段的工作，都应先建 task。先判定角色，再建 task，再写 prd.md。
**C 直接改动** — 只有用户明确要求跳过 Trellis 流程时才允许，例如“跳过 trellis”“直接改”“先别建任务”。
新增 task 时，task 标题、`prd.md`、`info.md`、`research/` 文档、`implement.jsonl` / `check.jsonl` 的说明文字默认必须使用中文；只有 Trellis、Codex、JSONL、PRD、C++、Blueprint、PIE、HUD 等必要专有名词可保留英文。
[/workflow-state:no_task]

### Phase 1: Plan
- 1.0 Create task `[required · once]`（只运行 `task.py create`；状态进入 planning）
- 1.1 Requirement exploration `[required · repeatable]`
- 1.2 Research `[optional · repeatable]`
- 1.3 Configure context `[required · once]` — Claude Code、Cursor、OpenCode、Codex、Kiro、Gemini、Qoder、CodeBuddy、Copilot、Droid、Pi
- 1.4 Activate task `[required · once]`（运行 `task.py start`；状态 → in_progress）
- 1.5 Completion criteria

<!-- Per-turn breadcrumb: shown throughout Phase 1 (status='planning') -->

[workflow-state:planning]
加载 `trellis-brainstorm` skill，并与用户迭代 `prd.md`。
先判定当前角色，再决定需要读取哪些 spec、旧 docs 和 task 文件。
双心印任务必须在 `prd.md` 中写清：当前目标、当前明确不做、验收口径，以及与当前阶段顺序的关系。
当前项目 task 粒度采用父 task + 子 task 混合模式：若当前任务过大，应继续拆子 task。
planning 阶段新增或改写的 task 文案默认必须使用中文，只有必要专有名词可保留英文；不要把整段需求、标题或确认问题写成英文。
完成后再 `task.py start <task-dir>` 进入 `in_progress`。
[/workflow-state:planning]

<!-- Per-turn breadcrumb: shown throughout Phase 1 when codex.dispatch_mode=inline.
     Codex-only opt-in alternate to [workflow-state:planning]. The main agent
     edits code directly in Phase 2, so Phase 1.3 jsonl curation is skipped —
     the inline workflow loads `trellis-before-dev` instead of injecting JSONL
     into a sub-agent. -->

[workflow-state:planning-inline]
加载 `trellis-brainstorm` skill，并与用户迭代 `prd.md`。
Codex inline 模式下不做 jsonl curation，但仍必须先判定角色、写清边界、确认 task 粒度是否合适。
task 标题和 task 内文默认必须中文化，不把用户将要看到的提示、问题或说明写成整段英文。
完成后再 `task.py start <task-dir>` 进入 `in_progress`。
[/workflow-state:planning-inline]

### Phase 2: Execute
- 2.1 Implement `[required · repeatable]`
- 2.2 Quality check `[required · repeatable]`
- 2.3 Rollback `[on demand]`

<!-- Per-turn breadcrumb: shown while status='in_progress'.
     Scope: all of Phase 2 + Phase 3.1-3.4 (status stays 'in_progress' from
     task.py start until task.py archive; only archive flips it). The body
     therefore must cover every required step from implementation through
     commit, including Phase 3.3 spec update and Phase 3.4 commit. -->

[workflow-state:in_progress]
**Flow**: 先按角色读 spec / task / 必要旧文档 → 实施或评估 → 测试/检查 → 回写 spec 或主文档 → commit → finish-work。
**Main-session default**: 当前项目使用 Codex inline 模式，主会话直接工作。
**双心印特殊要求**:
1. 写代码前必须已经获得明确指示
2. 若当前角色是主程序或设计师，默认优先写文档，不直接下沉写代码
3. 若当前角色是资深测试工程师，默认优先产出问题记录和回归结论，不直接改代码
4. 迁移期若新增稳定规则，应优先回写 `.trellis/spec`
5. task 内新增说明、检查结论、提交确认话术默认使用中文，只有必要专有名词可保留英文
[/workflow-state:in_progress]

<!-- Per-turn breadcrumb: shown while status='in_progress' when
     codex.dispatch_mode=inline. Codex-only opt-in alternate to
     [workflow-state:in_progress]. The main session edits code directly
     instead of dispatching sub-agents. -->

[workflow-state:in_progress-inline]
**Flow** (inline mode): 先判定角色 → 读取相关 spec / task / 必要旧文档 → main session 直接工作 → `trellis-check` → 回写 `.trellis/spec` 或 task / 主文档 → commit → `/trellis:finish-work`。
**Main-session default**: 主会话直接处理，不派子代理。
**双心印特殊要求**:
1. 开工前先判定角色
2. 任务过大时先拆父子 task
3. 写代码前需明确指示
4. 迁移期旧 `docs` 不删；后续持续维护以策划类文档为主
5. task 内文和面对用户的确认提示默认使用中文，不输出整段英文询问
[/workflow-state:in_progress-inline]

### Phase 3: Finish
- 3.1 Quality verification `[required · repeatable]`
- 3.2 Debug retrospective `[on demand]`
- 3.3 Spec update `[required · once]`
- 3.4 Commit changes `[required · once]`
- 3.5 Wrap-up reminder

<!-- Per-turn breadcrumb: shown while status='completed'.
     Currently DEAD in normal flow: cmd_archive writes status='completed' in
     the same call that moves the task dir to archive/, so the active-task
     resolver loses the pointer and the hook never fires on archived tasks.
     Block preserved for a future status-transition redesign (e.g. an
     explicit in_progress→completed command). Edit through the same spec
     channel as the live blocks. -->

[workflow-state:completed]
代码已在 Phase 3.4 提交；接下来运行 `/trellis:finish-work` 做收尾（archive task 并记录本次会话）。
若到达此状态时仍有未提交代码，先回到 Phase 3.4；`/finish-work` 不接受脏工作区。
`task.py archive` 会删除所有仍指向该已归档 task 的运行时会话文件。
[/workflow-state:completed]

### Rules

1. 先识别自己处于哪个 Phase，再继续执行该阶段的下一个步骤
2. 每个 Phase 内的步骤必须按顺序走；标记为 `[required]` 的步骤不能跳过
3. Phase 允许回退（例如 Execute 阶段发现 prd 有缺陷 → 回到 Plan 修 prd，再重新进入 Execute）
4. 标记为 `[once]` 的步骤，如果产物已存在则跳过，不要重复执行

### Skill Routing

当用户请求符合下列意图时，应先加载对应 skill（或派发对应子代理），不要跳过。

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

| User intent | Route |
|---|---|
| Wants a new feature / requirement unclear | `trellis-brainstorm` |
| About to write code / start implementing | Dispatch the `trellis-implement` sub-agent per Phase 2.1 |
| Finished writing / want to verify | Dispatch the `trellis-check` sub-agent per Phase 2.2 |
| Stuck / fixed same bug several times | `trellis-break-loop` |
| Spec needs update | `trellis-update-spec` |

**Why `trellis-before-dev` is NOT in this table:** you are not the one writing code — the `trellis-implement` sub-agent is. Sub-agent platforms get spec context via `implement.jsonl` injection / prelude, not via the main thread loading `trellis-before-dev`.

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-inline, Kilo, Antigravity, Windsurf]

| User intent | Skill |
|---|---|
| Wants a new feature / requirement unclear | `trellis-brainstorm` |
| About to write code / start implementing | `trellis-before-dev`（先判定角色并读取相关双心印 spec） |
| Finished writing / want to verify | `trellis-check` |
| Stuck / fixed same bug several times | `trellis-break-loop` |
| Spec needs update | `trellis-update-spec` |

[/codex-inline, Kilo, Antigravity, Windsurf]

### DO NOT skip skills

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

| What you're thinking | Why it's wrong |
|---|---|
| "This is simple, I'll just code it in the main thread" | 派发 `trellis-implement` 才是低成本正路；跳过它会诱导你在主线程写代码，失去 spec 上下文，而子代理会自动收到 `implement.jsonl` 注入 |
| "I already thought it through in plan mode" | plan 阶段的内容只在记忆里；子代理看不到，必须落到 `prd.md` |
| "I already know the spec" | spec 可能已经更新；子代理拿到的是新版本，你未必是 |
| "Code first, check later" | `trellis-check` 能发现你自己容易漏掉的问题，越早跑越省成本 |

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-inline, Kilo, Antigravity, Windsurf]

| What you're thinking | Why it's wrong |
|---|---|
| "This is simple, just code it" | 简单任务也很容易长复杂；`trellis-before-dev` 不到一分钟，但能把该读的 spec 先加载进来 |
| "I already thought it through in plan mode" | plan 阶段内容只存在于上下文记忆里，必须先落到 `prd.md` 再开工 |
| "I already know the spec" | spec 可能已经更新；请重新读一遍 |
| "Code first, check later" | `trellis-check` 能补上你注意不到的问题，越早越便宜 |

[/codex-inline, Kilo, Antigravity, Windsurf]

### 加载步骤详情

在任意步骤运行下面命令可取得详细说明：

```bash
python ./.trellis/scripts/get_context.py --mode phase --step <step>
# 例如：python ./.trellis/scripts/get_context.py --mode phase --step 1.1
```

---

## Phase 1: Plan

目标：搞清楚要做什么，产出清晰的需求文档与后续实施所需的上下文。

#### 1.0 Create task `[required · once]`

创建任务目录（状态进入 `planning`，且当会话身份可用时，会自动把该 task 设为当前活跃任务）：

```bash
python ./.trellis/scripts/task.py create "<task title>" --slug <name>
```

`--slug` 只填人类可读的名字，不要带 `MM-DD-` 日期前缀；`task.py create` 会自动补上。
默认优先使用中文标题；`--slug` 未显式传入时，也应优先由中文标题生成中文目录名。只有确实需要跨工具兼容或必须保留的专有名词时，才在标题或 slug 中使用英文。

命令成功后，每轮注入的 breadcrumb 会自动切换到 `[workflow-state:planning]`，驱动 AI 进入 brainstorm + jsonl curation 阶段。

⚠️ 这里只运行 `create`，不要顺手运行 `start`。`start` 会把状态切到 `in_progress`，导致 implementation 阶段的 breadcrumb 提前生效，brainstorm 与 jsonl curation 会被悄悄跳过。请把 `start` 留到 1.4，在 prd 与上下文都准备好之后再运行。

若 `python ./.trellis/scripts/task.py current --source` 已经指向某个 task，则跳过本步。

#### 1.1 Requirement exploration `[required · repeatable]`

加载 `trellis-brainstorm` skill，按其规则与用户一起梳理需求。

brainstorm skill 会要求你：
- 每次只问一个问题
- 能通过检索/阅读得到的答案就先自己去找，不先问用户
- 涉及偏好决策时尽量提供具体选项，而不是开放式提问
- 每次用户回复后立刻更新 `prd.md`

如果需求发生变化，就回到本步骤，继续修订 `prd.md`。

#### 1.2 Research `[optional · repeatable]`

研究可以在需求梳理过程中随时进行，不限于本地代码。你可以使用任何可用工具（MCP、skills、web search 等）查阅外部信息，包括第三方库文档、行业实践、API 参考等。

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

派发研究子代理：

- **Agent type**: `trellis-research`
- **Task description**: Research <specific question>
- **Key requirement**: 研究结果必须写入 `{TASK_DIR}/research/`

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-inline, Kilo, Antigravity, Windsurf]

直接在主会话中完成研究，并把结果写入 `{TASK_DIR}/research/`。（对 `codex-inline` 而言，这能避开 `fork_turns="none"` 带来的上下文隔离问题，避免研究子代理无法解析 active task 路径。）

[/codex-inline, Kilo, Antigravity, Windsurf]

**研究产物约定**：
- 每个研究主题一个文件（例如 `research/auth-library-comparison.md`）
- 文件中记录第三方库用法示例、API 引用、版本约束
- 顺手记下后续还需要参考的 spec 文件路径

brainstorm 与 research 可以自由交错：先暂停去研究一个技术问题，再回来继续收敛需求。

**核心原则**：研究结果必须落到文件里，不能只留在聊天上下文中。上下文会压缩，文件不会。

#### 1.3 Configure context `[required · once]`

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

整理 `implement.jsonl` 与 `check.jsonl`，让 Phase 2 的子代理拿到正确的 spec 上下文。这两个文件会在 `task create` 时先放一条 `_example` 种子行；你要做的是补成真实条目。

**位置**：`{TASK_DIR}/implement.jsonl` 与 `{TASK_DIR}/check.jsonl`（已存在）

**格式**：一行一个 JSON 对象：`{"file": "<path>", "reason": "<why>"}`。路径相对于仓库根目录。

**应该放什么**：
- **Spec 文件**：`.trellis/spec/<package>/<layer>/index.md` 及相关具体规范文件（如 `error-handling.md`、`conventions.md`）
- **Research 文件**：`{TASK_DIR}/research/*.md`

**不要放什么**：
- 代码文件（`src/**`、`packages/**/*.ts` 等）——这些由子代理在实施时自行读取
- 你马上就要修改的文件——原因同上

**两份文件如何分工**：
- `implement.jsonl` → 实施子代理写代码时需要的 spec + research
- `check.jsonl` → 检查子代理做质量复核时需要的 spec（必要时可带上同样的 research）

**如何发现相关 spec**：

```bash
python ./.trellis/scripts/get_context.py --mode packages
```

它会列出所有 package 与 spec layer 路径。按本 task 所在领域选取相关条目。

**如何追加条目**：

可以直接编辑 jsonl 文件，也可以运行：

```bash
python ./.trellis/scripts/task.py add-context "$TASK_DIR" implement "<path>" "<reason>"
python ./.trellis/scripts/task.py add-context "$TASK_DIR" check "<path>" "<reason>"
```

当真实条目已经存在后，可删除种子 `_example` 行（不删也不会影响消费者，它会自动忽略）。

若 `implement.jsonl` 中已经有 AI 整理过的真实条目（不只是种子行），则跳过本步。

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-inline, Kilo, Antigravity, Windsurf]

跳过本步。上下文会由 `trellis-before-dev` skill 在 Phase 2 中直接读取。

[/codex-inline, Kilo, Antigravity, Windsurf]

#### 1.4 Activate task `[required · once]`

当 `prd.md` 已完成，且 1.3 的 jsonl 整理也已完成后，把 task 状态切到 `in_progress`：

```bash
python ./.trellis/scripts/task.py start <task-dir>
```

成功后，breadcrumb 会自动切换到 `[workflow-state:in_progress]`，后续就进入 Phase 2 / 3。

如果 `task.py start` 因会话身份报错（hook 输入、`TRELLIS_CONTEXT_ID`、平台原生 session env 都没给上下文键），请按错误提示先补齐会话身份，再重试。

#### 1.5 Completion criteria

| Condition | Required |
|------|:---:|
| `prd.md` exists | ✅ |
| User confirms requirements | ✅ |
| `task.py start` has been run (status = in_progress) | ✅ |
| `research/` has artifacts (complex tasks) | recommended |
| `info.md` technical design (complex tasks) | optional |

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

| `implement.jsonl` has agent-curated entries (not just the seed row) | ✅ |

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

---

## Phase 2: Execute

目标：按 prd 落地改动，并通过质量检查。

#### 2.1 Implement `[required · repeatable]`

[Claude Code, Cursor, OpenCode, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

派发 implement 子代理：

- **Agent type**: `trellis-implement`
- **Task description**: Implement the requirements per prd.md, consulting materials under `{TASK_DIR}/research/`; finish by running project lint and type-check
- **Dispatch prompt guard**: 明确告诉被派发的代理，它已经是 `trellis-implement` 子代理，必须直接实施，不要再派发新的 `trellis-implement` / `trellis-check`

平台 hook / plugin 会自动：
- 读取 `implement.jsonl` 并把引用的 spec 文件注入子代理 prompt
- 注入 `prd.md` 内容

[/Claude Code, Cursor, OpenCode, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-sub-agent]

派发 implement 子代理：

- **Agent type**: `trellis-implement`
- **Task description**: Implement the requirements per prd.md, consulting materials under `{TASK_DIR}/research/`; finish by running project lint and type-check
- **Dispatch prompt guard**: prompt 必须以 `Active task: <task path>` 开头，并明确说明该代理已经是 `trellis-implement`，必须直接实施，不得再派发新的 `trellis-implement` / `trellis-check`

Codex 子代理定义会自动完成上下文载入：
- 通过 `task.py current --source` 解析当前活跃任务，并读取 `prd.md`、`info.md`（如果存在）
- 读取 `implement.jsonl`，要求代理在编码前先打开其中列出的每个 spec 文件

[/codex-sub-agent]

[Kiro]

派发 implement 子代理：

- **Agent type**: `trellis-implement`
- **Task description**: Implement the requirements per prd.md, consulting materials under `{TASK_DIR}/research/`; finish by running project lint and type-check
- **Dispatch prompt guard**: 明确告诉被派发的代理，它已经是 `trellis-implement`，必须直接实施，不要再派发新的 `trellis-implement` / `trellis-check`

平台 prelude 会自动：
- 读取 `implement.jsonl` 并将相关 spec 注入 prompt
- 注入 `prd.md`

[/Kiro]

[codex-inline, Kilo, Antigravity, Windsurf]

1. 加载 `trellis-before-dev` skill 以读取项目规范
2. 阅读 `{TASK_DIR}/prd.md` 明确需求
3. 查阅 `{TASK_DIR}/research/` 下的材料
4. 按要求直接实施
5. 运行项目 lint 与 type-check

[/codex-inline, Kilo, Antigravity, Windsurf]

#### 2.2 Quality check `[required · repeatable]`

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

派发 check 子代理：

- **Agent type**: `trellis-check`
- **Task description**: Review all code changes against spec and prd; fix any findings directly; ensure lint and type-check pass
- **Dispatch prompt guard**: 明确告诉被派发的代理，它已经是 `trellis-check`，必须直接审查 / 修复，不要再派发新的 `trellis-check` / `trellis-implement`

check 代理的职责：
- 按 spec 审查改动
- 自动修掉发现的问题
- 运行 lint 与 typecheck 验证

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi]

[codex-inline, Kilo, Antigravity, Windsurf]

加载 `trellis-check` skill，并按其说明完成验证：
- spec 合规性
- lint / type-check / tests
- 跨层一致性（若改动跨越多个 layer）

若发现问题 → 修复 → 重新检查，直到绿色。

[/codex-inline, Kilo, Antigravity, Windsurf]

#### 2.3 Rollback `[on demand]`

- `check` 暴露出 prd 缺陷 → 回到 Phase 1 修 `prd.md`，再重做 2.1
- 实施方向明显出错 → 回退代码，重新执行 2.1
- 需要更多研究 → 回到 1.2 的方式做研究，并把结果写入 `research/`

---

## Phase 3: Finish

目标：确认质量、沉淀经验、记录本次工作。

#### 3.1 Quality verification `[required · repeatable]`

加载 `trellis-check` skill，做最终质量验证：
- spec 合规性
- lint / type-check / tests
- 跨层一致性（若改动跨越多个 layer）

若发现问题 → 修复 → 重新检查，直到绿色。

#### 3.2 Debug retrospective `[on demand]`

如果本 task 里出现过重复调试（同类问题修了多次），加载 `trellis-break-loop` skill，完成：
- 根因分类
- 为什么之前的修法无效
- 如何避免再次发生

目标是把调试经验沉淀下来，避免同类问题继续循环。

#### 3.3 Spec update `[required · once]`

加载 `trellis-update-spec` skill，判断这次工作是否带来了值得写回 `.trellis/spec/` 的新知识：
- 新发现的模式或约定
- 踩过的坑
- 新技术决策

如有价值，就更新 `.trellis/spec/`；即使结论是“这次没有新规则可写回”，也要走完这个判断。

#### 3.4 Commit changes `[required · once]`

AI 负责驱动一次按批次的提交流程，确保 `/finish-work` 后续能干净执行。目标是：先提交本次工作改动，再做归档与日志类 bookkeeping 提交，绝不交错。

**步骤**：

1. **检查脏状态**：
   ```bash
   git status --porcelain
   ```
   记录所有脏文件路径。若工作区干净，直接跳到 3.5。

2. **观察提交风格**，让 commit message 与项目历史一致：
   ```bash
   git log --oneline -5
   ```
   关注前缀（`feat:` / `fix:` / `chore:` / `docs:` 等）、语言（优先中文）和长度风格。

3. **把脏文件分成两组**：
   - **本会话 AI 修改过的文件**：你本轮亲手改过，知道为什么改。
   - **无法识别的脏文件**：不是你这轮改的，可能是用户手改、上次遗留、或别的无关工作。**不要擅自纳入提交**。

4. **拟一份提交计划**。把 AI 改动按逻辑拆成 1 个或多个 commit（按变更单元分，不按文件分）。每个条目包含：`<commit message>` + 文件列表。无法识别的文件单列在最后。

5. **一次性把计划给用户确认**，格式如下：
   ```
   拟提交批次（按顺序）：
     1. <message>
        - <file>
        - <file>
     2. <message>
        - <file>

   未识别的脏文件（不会自动纳入任何提交，请确认是否包含）：
     - <file>
     - <file>

   回复“行”或“ok”即可执行；回复修改意见可调整文案；回复“我自己来”或“manual”则终止，由你手动提交。
   ```

6. **用户确认后**：按顺序执行 `git add <files>` + `git commit -m "<msg>"`。不要 amend，也不要 push。

7. **用户拒绝时**（例如回复“不行”“我自己来”“manual”，或对分组提出明显否定）：立刻停止，不要再试第二版计划。由用户手动提交；待用户确认后再继续 3.5。

**规则**：
- 全程不要使用 `git commit --amend`
- 此步骤绝不 push 到远端
- 若用户只想调整 message wording、但接受文件分组，可以改文案后再确认一次；若用户否定的是分组方式，就退出到 manual 模式
- 计划只发一轮，不按单个 commit 逐个征求同意
- 询问用户是否执行提交时，默认使用中文，不额外夹带英文整句提示

#### 3.5 Wrap-up reminder

完成上述步骤后，提醒用户可以运行 `/finish-work` 做最终收尾（归档 task、记录 session）。

---

## 自定义 Trellis（适用于 fork）

这一节面向想修改 Trellis 工作流本体的开发者。所有定制都通过编辑本文件完成；脚本只是解析器。

### 修改某个步骤的含义

直接编辑上方 Phase 1 / 2 / 3 中对应步骤的正文。**关键约束**：如果你改了某个步骤的 `[required · once]` 标记，或新增了 `[required · once]` 步骤，必须同步在对应阶段的 `[workflow-state:STATUS]` 标签块中补上对应的强化提示，否则每轮注入的 breadcrumb 不会提醒它，AI 就会静默跳过。回归测试会校验这一点。

4 个标签块都位于上方 `## Phase Index` 区域，分别紧跟在各阶段概览之后：

| 范围 | 对应标签 |
|---|---|
| 无活跃 task（Phase 1 之前） | `[workflow-state:no_task]`（位于 Phase Index ASCII 图下方） |
| 整个 Phase 1（task 创建后到准备实施前） | `[workflow-state:planning]`（位于 Phase 1 概览后） |
| Phase 2 + Phase 3.1～3.4（实施 + 检查 + 收尾） | `[workflow-state:in_progress]`（位于 Phase 2 概览后） |
| Phase 3.5 之后（已归档） | `[workflow-state:completed]`（位于 Phase 3 概览后，**当前不会触发**） |

### 修改每轮提示文字

直接编辑对应 `[workflow-state:STATUS]` 标签块的正文。改完后，如果你是模板维护者，请运行 `trellis update`；如果只是自定义当前项目，重启 AI 会话即可。无需改脚本。

### 添加自定义状态

新增一个块：

```
[workflow-state:my-status]
your per-turn prompt text
[/workflow-state:my-status]
```

约束：
- STATUS 字符集必须满足 `[A-Za-z0-9_-]+`（允许下划线和中横线，如 `in-review`、`blocked-by-team`）
- 必须有某个生命周期 hook 把 `task.json.status` 写成这个值，否则这个标签永远不会被读取
- 生命周期 hook 配置位于 `task.json.hooks.after_*`，可绑定 `after_create / after_start / after_finish / after_archive`

### 添加生命周期 Hook

在 `task.json` 中加入 `hooks` 字段：

```json
{
  "hooks": {
    "after_finish": [
      "your-script-or-command-here"
    ]
  }
}
```

支持的事件：`after_create / after_start / after_finish / after_archive`。注意 `after_finish` **不等于**状态变更（它只是清除 active-task 指针）；如果你想处理“任务完成”通知，请用 `after_archive`。

### 完整契约

关于工作流状态机的运行时契约、所有状态写入位置、伪状态（`no_task` / `stale_<source_type>`）、hook 可达性矩阵等更底层的细节，请看：

- `.trellis/spec/cli/backend/workflow-state-contract.md` — 运行时契约、写入者列表、测试不变量
- `.trellis/scripts/inject-workflow-state.py` — 实际解析器（只读 workflow.md，不内置文本）
