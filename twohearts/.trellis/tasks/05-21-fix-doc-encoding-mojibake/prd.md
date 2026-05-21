# 修复 AI 读取中文文档时的乱码问题

## Goal

降低 AI 在 Windows 环境下读取项目中文文档时误判乱码的概率，让 `.trellis/`、`.agents/`、`AGENTS.md` 与旧 `docs/` 中的高频文档都能被稳定读取。

## What I already know

* 当前环境是 Windows PowerShell 5.1，控制台代码页为 `936`
* 项目中的 Markdown 文档大多是 `UTF-8` 无 BOM
* 在当前环境下，默认读取这类文件时容易出现乱码；显式 `-Encoding UTF8` 读取则正常
* Python 标准输出当前为 `gbk`，直接打印中文文档内容也可能出现乱码

## Requirements

* 统一修复仓库内高频中文 Markdown 文档的编码兼容性
* 补充明确规则，要求在 Windows 下读取中文文档时优先使用安全的 UTF-8 读取方式
* 尽量不改变文档语义，必要时允许只做格式与编码层调整

## Acceptance Criteria

* [ ] 抽样文档在 PowerShell 默认 `Get-Content` 读取下不再出现乱码
* [ ] 仓库中有明确规则说明如何稳定读取中文文档
* [ ] 本次修复不修改业务代码逻辑

## Out of Scope

* 修改业务功能代码
* 重写大段现有文档内容
* 处理非 Markdown 文件的全部编码策略

## Technical Notes

* 优先处理 `.trellis/**/*.md`、`.agents/**/*.md`、`AGENTS.md`、`../docs/**/*.md`
* 可通过统一转为 `UTF-8 with BOM` 改善 PowerShell 5.1 默认读取兼容性
* 规则说明中应覆盖 PowerShell 和 Python 两条常见读取链路
