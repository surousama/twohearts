# Trellis 关键位置导航

## 1. 文档用途

本文件用于集中说明双心印项目中 Trellis 的关键目录、关键文件和常用查看入口。

当你后续要看“稳定规则在哪”“工作流在哪”“任务在哪”“怎么查当前上下文”时，优先从这里定位。

## 2. Trellis 总入口

### 2.1 工作流总说明

文件：[`H:\twohearts\twohearts\.trellis\workflow.md`](../twohearts/.trellis/workflow.md)

作用：

1. 说明 Trellis 的完整开发流程。
2. 定义 `Phase 1 / 2 / 3` 的含义和顺序。
3. 说明什么时候该建 task，什么时候该实现，什么时候该检查和收尾。
4. 也是运行时会被读取的工作流来源之一，不只是普通说明文档。

什么时候看：

1. 想确认当前应该走哪一步时。
2. 想调整 Trellis 工作流文本时。
3. 想看 task 生命周期和收尾流程时。

### 2.2 项目级配置

文件：[`H:\twohearts\twohearts\.trellis\config.yaml`](../twohearts/.trellis/config.yaml)

作用：

1. 存放 Trellis 的项目级开关配置。
2. 控制 session 自动提交、journal 行数上限、Codex 分发模式等行为。

当前关键配置：

1. `session_auto_commit: false`
2. `codex.dispatch_mode: inline`

什么时候看：

1. 想改 Trellis 的运行方式时。
2. 想确认当前是不是 `inline` 模式时。
3. 想查 session/journal 的默认策略时。

## 3. 稳定规则看哪里

### 3.1 总规则根目录

目录：[`H:\twohearts\twohearts\.trellis\spec\`](../twohearts/.trellis/spec)

作用：

1. 这里是 Trellis 里的稳定规则主区。
2. 后续项目稳定约定，原则上优先沉淀在这里，而不是继续散落在旧 `docs`。

### 3.2 工作流规则

文件：[`H:\twohearts\twohearts\.trellis\spec\workflow\index.md`](../twohearts/.trellis/spec/workflow/index.md)

作用：

1. 管 AI 的默认工作方式。
2. 管角色判定、阅读范围、迁移期规则。

适合查看的问题：

1. AI 回答和阅读范围应该遵守什么规则？
2. 迁移期旧 `docs`` 是否还能保留？
3. 角色先判定还是先实现？

### 3.3 任务规则

文件：[`H:\twohearts\twohearts\.trellis\spec\tasking\index.md`](../twohearts/.trellis/spec/tasking/index.md)

配套文件：

1. [`parent-child-task-rules.md`](../twohearts/.trellis/spec/tasking/parent-child-task-rules.md)
2. [`prd-rules.md`](../twohearts/.trellis/spec/tasking/prd-rules.md)

作用：

1. 说明 task 粒度怎么拆。
2. 说明父 task 和子 task 怎么分工。
3. 说明 `prd.md` 应该写什么。

适合查看的问题：

1. 现在该不该拆子 task？
2. 一个 task 怎样才算可单轮验收？
3. `prd.md` 该写哪些内容？

### 3.4 角色规则

文件：[`H:\twohearts\twohearts\.trellis\spec\roles\index.md`](../twohearts/.trellis/spec/roles/index.md)

作用：

1. 说明游戏设计师、主程序、资深程序、资深测试工程师各自的职责边界。
2. 避免 AI 一轮里混着扮演多个角色。

### 3.5 项目级规则

文件：[`H:\twohearts\twohearts\.trellis\spec\project\index.md`](../twohearts/.trellis/spec/project/index.md)

作用：

1. 说明双心印是什么项目。
2. 说明当前开发阶段和项目推进顺序。
3. 防止 AI 偏离双人共斗、结印、合印这些核心方向。

### 3.6 实现规则

文件：[`H:\twohearts\twohearts\.trellis\spec\implementation\index.md`](../twohearts/.trellis/spec/implementation/index.md)

作用：

1. 说明 `UE5 + GAS + C++` 路线下的实现约束。
2. 说明蓝图与 C++ 的边界。
3. 说明文档回写和正式承载位置规则。

### 3.7 测试规则

文件：[`H:\twohearts\twohearts\.trellis\spec\testing\index.md`](../twohearts/.trellis/spec/testing/index.md)

作用：

1. 说明跑测、白盒、问题记录和测试文档维护规则。

### 3.8 通用思考指南

文件：[`H:\twohearts\twohearts\.trellis\spec\guides\index.md`](../twohearts/.trellis/spec/guides/index.md)

作用：

1. 不是项目硬规则，而是开工前的补充思考清单。
2. 适合查代码复用、跨层一致性、阶段顺序判断。

## 4. 任务看哪里

### 4.1 当前与历史任务目录

目录：[`H:\twohearts\twohearts\.trellis\tasks\`](../twohearts/.trellis/tasks)

作用：

1. 存放当前 active task 和历史 task。
2. 每个 task 都是一个独立目录。

每个 task 常见文件：

1. `prd.md`：需求和边界
2. `task.json`：任务元数据
3. `implement.jsonl`：实现时要注入哪些规范
4. `check.jsonl`：检查时要注入哪些规范
5. `research/`：研究资料
6. `info.md`：可选的技术补充说明

### 4.2 归档任务

目录：[`H:\twohearts\twohearts\.trellis\tasks\archive\`](../twohearts/.trellis/tasks/archive)

作用：

1. 存放已归档任务。
2. 适合回看历史 PRD、研究记录和阶段结论。

## 5. 会话与日志看哪里

### 5.1 工作区总目录

目录：[`H:\twohearts\twohearts\.trellis\workspace\`](../twohearts/.trellis/workspace)

作用：

1. 存放开发者的 session 和 journal 记录。
2. 用于跨会话追踪工作历史。

### 5.2 当前开发者记录

目录：[`H:\twohearts\twohearts\.trellis\workspace\palladianli\`](../twohearts/.trellis/workspace/palladianli)

作用：

1. 查看当前开发者的 `index.md` 和 `journal-N.md`。
2. 了解最近进行了哪些 Trellis 会话。

## 6. 运行时状态看哪里

### 6.1 当前会话状态

目录：[`H:\twohearts\twohearts\.trellis\.runtime\sessions\`](../twohearts/.trellis/.runtime/sessions)

作用：

1. 存放当前 session 对应的 task 指针。
2. 用来判断当前会话激活的是哪个 task。

适合查看的问题：

1. 为什么当前没有 active task？
2. `task.py start` 之后到底有没有写入当前会话状态？

## 7. 常用脚本入口看哪里

### 7.1 任务脚本

文件：[`H:\twohearts\twohearts\.trellis\scripts\task.py`](../twohearts/.trellis/scripts/task.py)

作用：

1. 创建、启动、归档、查询 task。
2. 管理 task 元数据和上下文文件。

### 7.2 上下文脚本

文件：[`H:\twohearts\twohearts\.trellis\scripts\get_context.py`](../twohearts/.trellis/scripts/get_context.py)

作用：

1. 查询当前 session 上下文。
2. 查询工作流阶段说明。
3. 查询 package / spec layer 信息。

### 7.3 开发者脚本

文件：

1. [`init_developer.py`](../twohearts/.trellis/scripts/init_developer.py)
2. [`get_developer.py`](../twohearts/.trellis/scripts/get_developer.py)
3. [`add_session.py`](../twohearts/.trellis/scripts/add_session.py)

作用：

1. 初始化开发者身份。
2. 查询当前开发者。
3. 记录 session。

## 8. 最常用的查找顺序

如果只是日常使用 Trellis，通常按下面顺序找就够了：

1. 想看整体流程：先看 `.trellis/workflow.md`
2. 想看稳定规则：先看 `.trellis/spec/`
3. 想看任务要求：先看对应 task 下的 `prd.md`
4. 想看当前项目阶段：看 `.trellis/spec/project/`
5. 想看 task 规范：看 `.trellis/spec/tasking/`
6. 想看当前会话挂的是哪个任务：看 `.trellis/.runtime/sessions/`
7. 想查命令怎么用：看 `.trellis/scripts/task.py` 或直接跑 `--help`

## 9. 当前双心印项目的使用结论

1. Trellis 的工作流总入口在 `.trellis/workflow.md`。
2. 稳定规则主区在 `.trellis/spec/`。
3. 双心印项目最常用的规则层是 `workflow`、`tasking`、`roles`、`project`、`implementation`。
4. 当前 Trellis 运行模式是 `Codex inline`。
5. 迁移期内旧 `docs` 继续保留，不删除，但后续稳定规则优先沉淀到 `.trellis/spec/`。
