# Trellis 配置梳理与中文化

## Goal

检查当前项目内 Trellis 的配置、规范入口和任务机制是否已经处于可用状态；在不影响运行功能的前提下，将 `.trellis` 中面向使用者的英文说明文档和提示文本尽量改为中文，并整理出一份适合后续查看的关键配置导航。

## What I already know

* 当前项目已完成 Trellis 初步安装与基础接入。
* 当前仓库内 `.trellis/` 已存在 `workflow.md`、`config.yaml`、`spec/`、`scripts/`、`tasks/`、`workspace/` 等核心结构。
* `config.yaml` 当前启用了 `codex.dispatch_mode: inline`。
* `.trellis/spec/workflow/` 与 `.trellis/spec/tasking/` 等双心印项目规范已经是中文。
* 当前更可能需要汉化的是 `.trellis` 根目录中的通用说明、工作流说明，以及部分任务模板或注释性文本。

## Requirements

* 盘点当前 `.trellis` 的关键配置和目录结构，确认哪些内容已配置完成、哪些内容仍需关注。
* 在不影响 Trellis 功能的前提下，将 `.trellis` 中面向使用者的英文文档、注释、提示说明改为中文。
* 不修改会影响运行契约的关键标识，例如命令名、状态名、标签名、脚本约定字段、JSON/YAML 键名。
* 如发现文档编码、乱码或明显可用性问题，一并修复到可读状态。
* 完成后输出一份 Trellis 关键配置导航，说明稳定规则、工作流、任务、规范、脚本入口分别在哪里查看。

## Acceptance Criteria

* [ ] 已梳理出当前 `.trellis` 的关键配置状态。
* [ ] `.trellis` 中主要面向使用者的英文说明文档已改为中文，且不破坏现有功能。
* [ ] `workflow.md` 等关键文件在中文环境下可正常阅读，不存在明显乱码。
* [ ] 已整理出一份 Trellis 关键配置与查看路径清单。

## Out of Scope

* 修改 Trellis Python 脚本的业务逻辑
* 重构 `.trellis/spec` 的规则体系
* 迁移项目业务代码或 `docs/` 目录内容
* 对所有历史 task 文档做统一翻译

## Technical Notes

* 当前任务目录：`.trellis/tasks/05-20-trellis-config-zh-audit/`
* 当前重点检查文件：`.trellis/config.yaml`、`.trellis/workflow.md`、`.trellis/spec/**/index.md`
* 运行安全边界：只翻译说明文本，不改动会被脚本解析的结构化标记
