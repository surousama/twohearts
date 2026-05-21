# 优化 VS Code Unreal 工作区性能

## Goal

在不影响当前 `twohearts` 项目编译、跳转和日常 Unreal C++ 开发体验的前提下，降低 VS Code 在打开工作区时的文件枚举、C/C++ 扫描和文件监视负担，消除或显著缓解当前由超大工作区根目录引发的性能警告。

## What I already know

* 当前 VS Code 弹出提示：已枚举约 35.9 万个文件，检测到约 25.8 万个 C/C++ 源文件。
* 现有 [`twohearts.code-workspace`](H:/twohearts/twohearts/twohearts.code-workspace:1) 将 `H:\UE_5.6` 作为单独工作区根目录加入。
* 现有 `.vscode/settings.json` 和 `.code-workspace` 已排除 `Binaries`、`DerivedDataCache`、`Intermediate`、`Saved`，但尚未配置 `files.watcherExclude`。
* 现有 `.vscode/c_cpp_properties.json` 通过 `compile_commands.json`、`includePath` 和 `browse.path` 指向项目源码与 UE5 引擎头文件。
* 用户已选择“激进优化”：移除 `UE5` 工作区根目录，只保留项目根目录和现有编译数据库链路。

## Assumptions (temporary)

* Unreal C++ 的头文件解析、跳转与 IntelliSense 主要依赖 `compile_commands.json`、`includePath` 和 `browse.path`，不要求将整个引擎目录暴露为工作区根目录。
* 用户更看重 VS Code 整体响应和索引性能，而不是在资源管理器里直接浏览 `H:\UE_5.6` 全量目录。

## Open Questions

* 无阻塞问题，按已确认方案直接实施。

## Requirements (evolving)

* 从工作区移除 `H:\UE_5.6` 这个单独的工作区根目录。
* 保留当前基于 `compile_commands.json`、编译器路径和 UE5 头文件路径的 C/C++ 开发能力。
* 为工作区补充更完整的 VS Code 文件监视排除规则，减少不必要的 watcher 压力。
* 补强 `C_Cpp.files.exclude`，避免 C/C++ 扩展继续扫描明显无关的大目录或缓存目录。
* 不修改项目源码、构建脚本、Unreal 引擎安装内容。

## Acceptance Criteria (evolving)

* [ ] [`twohearts.code-workspace`](H:/twohearts/twohearts/twohearts.code-workspace:1) 不再包含 `UE5` 工作区根目录。
* [ ] 工作区仍保留现有 `compile_commands.json` / 编译器 / IntelliSense 基础配置。
* [ ] 工作区新增 `files.watcherExclude`，覆盖 Unreal 常见高噪音目录。
* [ ] `C_Cpp.files.exclude` 比当前更完整，并与“移除 UE5 根目录”的策略一致。
* [ ] 本次修改仅影响 VS Code 工作区和编辑器配置文件，可单独回滚与验收。

## Definition of Done (team quality bar)

* 变更范围清晰且仅限工作区配置文件
* 不破坏现有 Unreal VS Code 构建与打开编辑器脚本入口
* 风险和后续可选优化在任务结尾说明清楚

## Out of Scope (explicit)

* 调整 Unreal 项目源码、模块结构或插件布局
* 修改 `Scripts/Invoke-UE.ps1` 等构建脚本逻辑
* 清理或重构 `H:\UE_5.6` 引擎安装目录
* 更换 C/C++ 扩展、改用 `clangd`、迁移 IDE

## Technical Notes

* 角色判定：本任务按“资深程序”执行，聚焦工具链与实现层配置，不改设计文档。
* 任务粒度判定：这是一个可单独验收的小型工程效率任务，适合作为独立 task 处理。
* 关键现状文件：
  * [`twohearts.code-workspace`](H:/twohearts/twohearts/twohearts.code-workspace:1)
  * [`.vscode/settings.json`](H:/twohearts/twohearts/.vscode/settings.json:1)
  * [`.vscode/c_cpp_properties.json`](H:/twohearts/twohearts/.vscode/c_cpp_properties.json:1)
  * [`.ignore`](H:/twohearts/twohearts/.ignore:1)
