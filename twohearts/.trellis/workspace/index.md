# 工作区索引

> 记录本项目中所有开发者的 AI Agent 工作记录

---

## 概览

这个目录用于跟踪所有使用 AI Agent 参与本项目的开发者记录。

### 文件结构

```
workspace/
|-- index.md              # 本文件 - 总索引
+-- {developer}/          # 每位开发者一个目录
    |-- index.md          # 个人索引与会话历史
    |-- tasks/            # 任务文件
    |   |-- *.json        # 活跃任务
    |   +-- archive/      # 按月份归档的历史任务
    +-- journal-N.md      # 日志文件（顺序编号：1、2、3...）
```

---

## 当前活跃开发者

| 开发者 | 最近活跃 | 会话数 | 当前文件 |
|-----------|-------------|----------|-------------|
| （暂无） | - | - | - |

---

## 快速开始

### 新开发者

运行初始化脚本：

```bash
python ./.trellis/scripts/init_developer.py <your-name>
```

这会：
1. 创建你的身份文件（已 gitignore）
2. 创建你的工作目录
3. 创建你的个人索引
4. 创建初始 journal 文件

### 回归开发者

1. 获取你的开发者名称：
   ```bash
   python ./.trellis/scripts/get_developer.py
   ```

2. 阅读你的个人索引：
   ```bash
   cat .trellis/workspace/$(python ./.trellis/scripts/get_developer.py)/index.md
   ```

---

## 约定

### Journal 文件规则

- 单个 journal 文件**最多 2000 行**
- 达到上限后，创建 `journal-{N+1}.md`
- 创建新文件时，同步更新你的个人 `index.md`

### 会话记录格式

每次 session 应包含：
- 概要：一句话说明本次工作
- 分支：工作所在分支
- 主要改动：本次主要修改内容
- Git 提交：相关提交 hash 与 message
- 下一步：后续计划

---

## Session 模板

记录 session 时可使用以下模板：

```markdown
## Session {N}: {Title}

**日期**: YYYY-MM-DD
**任务**: {task-name}
**分支**: `{branch-name}`

### 概要

{一句话总结}

### 主要改动

- {改动 1}
- {改动 2}

### Git 提交

| Hash | Message |
|------|---------|
| `abc1234` | {commit message} |

### 测试

- [OK] {测试结果}

### 状态

[OK] **已完成** / [~] **进行中** / [P] **阻塞**

### 下一步

- {下一步 1}
- {下一步 2}
```

---

**语言约定**：文档默认使用中文；如需保留英文术语，应以便于项目协作为准。
