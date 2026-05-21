# Trellis 使用指南

## 1. 文档用途

本文件用于集中说明双心印项目里 Trellis 的入口位置、推荐开场方式、初始化步骤、常用命令和日常排查方法。

如果你后续想确认下面这些问题，优先先看这里：

1. Trellis 工作流总入口在哪
2. 新对话怎么开场，才能正确进入工作流
3. 当前 task、spec、会话日志分别放哪里
4. 为什么当前没有 `active task`
5. 日常最常用的命令有哪些

## 2. 当前维护口径

### 2.1 `.trellis/` 与 `docs/` 的分工

1. `.trellis/` 是当前 Trellis 工作流的主工作区。
2. 稳定规则优先维护在 [`.trellis/spec/`](../twohearts/.trellis/spec)。
3. task、PRD、研究记录、质量检查过程优先维护在 [`.trellis/tasks/`](../twohearts/.trellis/tasks)。
4. `docs/` 目录继续保留，但主要维护策划类文档和长期项目背景文档。
5. `docs/` 下的开发任务文档、程序实施过程文档、测试文档不再作为持续维护主区，后续以留档回看为主。

### 2.2 这条分工意味着什么

1. 想看当前 AI 工作流和项目规则，优先看 `.trellis/`。
2. 想保留策划层的长期说明、产品背景、玩法方向，仍可以维护 `docs/`。
3. 想写当前 task 的需求、研究、实现过程、检查记录，不要再继续往旧 `docs/测试文档` 或 `docs/程序技术文档` 里堆。

## 3. 新对话如何正确进入 Trellis 工作流

### 3.1 推荐开场模板

在新对话里，最推荐直接使用下面这段：

```text
进入 Trellis 工作流。
我的目标是：<目标>。
这次希望你：<直答 / 继续已有 task / 创建新 task / 跳过 Trellis 直接改>。
相关入口：<task 名 / 文件路径 / 背景说明>。
```

这段话的作用是让 AI 一开始就明确四件事：

1. 这次要不要进入 Trellis
2. 目标是什么
3. 你希望它走哪种路由
4. 它应该先读哪个入口

### 3.2 四种常见开场方式

1. 正式推进新任务：

```text
进入 Trellis 工作流。
我的目标是：推进第二章基础战斗模块的新需求。
这次希望你：创建新 task。
相关入口：先检查当前 active task 和相关 spec。
```

2. 继续已有 task：

```text
进入 Trellis 工作流。
我的目标是：继续基础闪避第二轮收口。
这次希望你：继续已有 task。
相关入口：05-19-dodge-second-pass-polish。
```

3. 只做阅读或解释：

```text
进入 Trellis 工作流。
我的目标是：了解某个 task 或规范。
这次希望你：直答。
相关入口：对应的 task 文件或 spec 文件。
```

4. 明确跳过 Trellis，直接改：

```text
跳过 Trellis。
直接修改 <文件或功能>。
```

### 3.3 一个关键提醒

1. 如果只是问“帮我看看”“解释一下”“先读一下”，通常属于直答。
2. 如果要落文档、改配置、改代码、做测试计划、做技术评估、推进真实开发阶段，通常应进入正式 task 流程。
3. 如果你已经知道要继续哪个 task，最好直接把 task 名写出来。

## 4. 第一次使用 Trellis

### 4.1 初始化开发者身份

先运行：

```powershell
python .\.trellis\scripts\init_developer.py palladianli
```

如果你想换成自己的名字，把 `palladianli` 替换掉即可。

它会做两件事：

1. 创建 `.trellis/.developer`
2. 让 Trellis 知道当前默认开发者是谁，并使用对应的 `workspace/<name>/`

### 4.2 检查开发者身份是否生效

```powershell
python .\.trellis\scripts\get_developer.py
python .\.trellis\scripts\get_context.py
```

如果仍然看到 `Developer not initialized`，说明当前身份还没成功写入。

### 4.3 第一次正式试跑的推荐顺序

```powershell
python .\.trellis\scripts\init_developer.py palladianli
python .\.trellis\scripts\get_developer.py
python .\.trellis\scripts\get_context.py
python .\.trellis\scripts\task.py list
```

然后再根据情况选择：

1. 继续一个已有 task：运行 `task.py start <task目录名>`
2. 新建一个 task：先运行 `task.py create ...`，再运行 `task.py start ...`

### 4.4 让运行时 task 绑定机制生效

先确保已经初始化开发者身份，然后执行：

```powershell
python .\.trellis\scripts\task.py start <task目录名>
python .\.trellis\scripts\task.py current --source
```

正常情况下会发生：

1. 当前会话拿到 `active task`
2. [`.trellis/.runtime/sessions/`](../twohearts/.trellis/.runtime/sessions) 开始生成运行时指针文件
3. `get_context.py` 能读到当前任务上下文

如果 `task.py start` 成功了，但 `.runtime/sessions/` 仍然没有出现，就继续检查：

1. 当前是否已经执行过 `init_developer.py`
2. 当前是否真的是同一个终端会话
3. 当前平台是否给了 Trellis 可识别的会话上下文

## 5. Trellis 核心入口

### 5.1 工作流总说明

文件：[`.trellis/workflow.md`](../twohearts/.trellis/workflow.md)

作用：

1. 说明 Trellis 的完整开发流程
2. 定义 `Phase 1 / 2 / 3` 的含义和顺序
3. 说明什么时候该建 task，什么时候该实现，什么时候该检查和收尾
4. 它也是运行时会被读取的工作流来源之一，不只是普通说明文档

### 5.2 项目级配置

文件：[`.trellis/config.yaml`](../twohearts/.trellis/config.yaml)

作用：

1. 存放 Trellis 的项目级开关配置
2. 控制 session 自动提交、journal 行数上限、Codex 分发模式等行为

当前关键配置：

1. `session_auto_commit: false`
2. `codex.dispatch_mode: inline`

### 5.3 稳定规则主区

目录：[`.trellis/spec/`](../twohearts/.trellis/spec)

常用入口：

1. 工作流规则：[`.trellis/spec/workflow/index.md`](../twohearts/.trellis/spec/workflow/index.md)
2. task 规则：[`.trellis/spec/tasking/index.md`](../twohearts/.trellis/spec/tasking/index.md)
3. 角色规则：[`.trellis/spec/roles/index.md`](../twohearts/.trellis/spec/roles/index.md)
4. 项目规则：[`.trellis/spec/project/index.md`](../twohearts/.trellis/spec/project/index.md)
5. 实现规则：[`.trellis/spec/implementation/index.md`](../twohearts/.trellis/spec/implementation/index.md)
6. 测试规则：[`.trellis/spec/testing/index.md`](../twohearts/.trellis/spec/testing/index.md)
7. 通用思考指南：[`.trellis/spec/guides/index.md`](../twohearts/.trellis/spec/guides/index.md)

## 6. task、会话和运行时状态看哪里

### 6.1 当前与历史 task

目录：[`.trellis/tasks/`](../twohearts/.trellis/tasks)

每个 task 常见文件：

1. `prd.md`：需求和边界
2. `task.json`：任务元数据
3. `implement.jsonl`：实现阶段上下文
4. `check.jsonl`：检查阶段上下文
5. `research/`：研究资料
6. `info.md`：可选技术补充说明

归档目录：[`.trellis/tasks/archive/`](../twohearts/.trellis/tasks/archive)

### 6.2 会话日志

目录：[`.trellis/workspace/`](../twohearts/.trellis/workspace)

如果你要看 `palladianli` 的记录，可直接打开：
[`.trellis/workspace/palladianli/`](../twohearts/.trellis/workspace/palladianli)

### 6.3 运行时状态

目录：[`.trellis/.runtime/sessions/`](../twohearts/.trellis/.runtime/sessions)

作用：

1. 存放当前 session 对应的 task 指针
2. 用来判断当前会话激活的是哪个 task
3. 如果这里还没生成，通常说明当前会话还没完成开发者初始化，或者还没成功执行 `task.py start`

## 7. 常用命令

### 7.1 看当前上下文

```powershell
python .\.trellis\scripts\get_context.py
python .\.trellis\scripts\get_context.py --mode phase
python .\.trellis\scripts\get_context.py --mode phase --step 1.1
python .\.trellis\scripts\get_context.py --mode packages
```

适合用途：

1. 看当前开发者和 task 状态
2. 看当前该走哪个 Phase
3. 看某一步的详细说明
4. 看当前 spec layer 结构

### 7.2 常用 task 命令

```powershell
python .\.trellis\scripts\task.py list
python .\.trellis\scripts\task.py current --source
python .\.trellis\scripts\task.py create "任务标题" --slug readable-name --assignee palladianli
python .\.trellis\scripts\task.py start 05-20-some-task
python .\.trellis\scripts\task.py finish
python .\.trellis\scripts\task.py archive 05-20-some-task
python .\.trellis\scripts\task.py list-archive
```

说明：

1. `create`：创建 task 目录，进入 `planning`
2. `start`：把 task 切到 `in_progress`，并尝试写当前会话指针
3. `finish`：清除当前会话的 `active task` 指针，但不归档
4. `archive`：把 task 移到归档目录

### 7.3 task 上下文命令

```powershell
python .\.trellis\scripts\task.py add-context 05-20-some-task implement .trellis/spec/workflow/index.md "工作流规则"
python .\.trellis\scripts\task.py add-context 05-20-some-task check .trellis/spec/testing/index.md "检查规则"
python .\.trellis\scripts\task.py list-context 05-20-some-task
python .\.trellis\scripts\task.py validate 05-20-some-task
```

适合用途：

1. 给实现阶段补上下文
2. 给检查阶段补上下文
3. 核对 `JSONL` 是否有效

### 7.4 session 与日志命令

```powershell
python .\.trellis\scripts\add_session.py --title "本次会话标题" --commit "hash" --summary "一句话总结"
```

适合用途：

1. 给当前开发者工作区追加会话记录
2. 让 `workspace/<developer>/journal-N.md` 和 `index.md` 更新

### 7.5 开发者命令

```powershell
python .\.trellis\scripts\init_developer.py palladianli
python .\.trellis\scripts\get_developer.py
```

## 8. 最常用的查看顺序

如果只是日常使用 Trellis，通常按下面顺序找就够了：

1. 想看整体流程：先看 [`.trellis/workflow.md`](../twohearts/.trellis/workflow.md)
2. 想看稳定规则：先看 [`.trellis/spec/`](../twohearts/.trellis/spec)
3. 想看任务要求：先看对应 task 下的 `prd.md`
4. 想看当前项目阶段：看 [`.trellis/spec/project/index.md`](../twohearts/.trellis/spec/project/index.md)
5. 想看 task 规范：看 [`.trellis/spec/tasking/index.md`](../twohearts/.trellis/spec/tasking/index.md)
6. 想看当前会话挂的是哪个任务：看 [`.trellis/.runtime/sessions/`](../twohearts/.trellis/.runtime/sessions)
7. 想查命令怎么用：看 [`.trellis/scripts/task.py`](../twohearts/.trellis/scripts/task.py) 或直接跑 `--help`

## 9. 常见排查

### 9.1 `Developer not initialized`

先运行：

```powershell
python .\.trellis\scripts\init_developer.py palladianli
```

### 9.2 `Current task: (none)`

常见原因：

1. 还没执行 `task.py start`
2. 当前会话没有 developer 身份
3. 当前终端没有拿到可识别的 session 上下文

排查顺序：

```powershell
python .\.trellis\scripts\get_developer.py
python .\.trellis\scripts\task.py current --source
python .\.trellis\scripts\task.py list
```

### 9.3 `.trellis/.runtime/sessions/` 没生成

先检查：

1. 是否已经执行 `init_developer.py`
2. 是否已经执行 `task.py start <task>`
3. 是否是同一轮会话上下文

## 10. 当前双心印项目的结论

1. Trellis 的工作流总入口在 [`.trellis/workflow.md`](../twohearts/.trellis/workflow.md)。
2. 稳定规则主区在 [`.trellis/spec/`](../twohearts/.trellis/spec)。
3. 双心印项目最常用的规则层是 `workflow`、`tasking`、`roles`、`project`、`implementation`。
4. 当前 Trellis 运行模式是 `Codex inline`。
5. `docs/` 目录会继续维护，但主要保留策划类和长期背景类文档；开发任务、测试过程、程序实施过程文档以后以 Trellis task 和 spec 为主。
