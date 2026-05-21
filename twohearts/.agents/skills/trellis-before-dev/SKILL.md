---
name: trellis-before-dev
description: "Discovers and injects project-specific coding guidelines from .trellis/spec/ before implementation begins. Reads spec indexes, pre-development checklists, and shared thinking guides for the target package. Use when starting a new coding task, before writing any code, switching to a different package, or needing to refresh project conventions and standards."
---

在双心印项目中，写代码前必须先读取相关规范。

执行步骤：

1. **先判定当前角色**
   - 游戏设计师
   - 主程序
   - 资深程序
   - 资深测试工程师

2. **查看可用 spec 层**
   ```bash
   python ./.trellis/scripts/get_context.py --mode packages
   ```

3. **至少读取以下索引**
   ```bash
   cat .trellis/spec/workflow/index.md
   cat .trellis/spec/roles/index.md
   cat .trellis/spec/guides/index.md
   ```

4. **按角色补读**
   - 设计师：`project`、`workflow`、`roles`、`combat`
   - 主程序：`project`、`workflow`、`roles`、`combat`、`implementation`、`tasking`
   - 资深程序：`workflow`、`roles`、`combat`、`implementation`、`tasking`
   - 测试：`workflow`、`roles`、`testing`、`tasking`

5. **按当前 task 补读**
   - 读取当前 task 的 `prd.md`
   - 若迁移期仍依赖旧 `docs`，只读取当前任务直接相关的旧文档

6. **确认边界**
   - 当前目标是什么
   - 当前明确不做什么
   - 当前验收口径是什么

在双心印项目中，这一步在写任何代码前都是强制的。
