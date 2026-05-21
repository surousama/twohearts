# 统一脚本 UTF-8 输出链路

## Goal

让双心印项目内高频 Trellis 脚本在 Windows 环境下输出中文时更稳定，减少 PowerShell 5.1、Python 标准流和子进程链路中的乱码概率。

## What I already know

* 之前的 Markdown 文档已经统一为 `UTF-8 with BOM`
* `.trellis/scripts/common/__init__.py` 已经会为 Python 标准流重配 `utf-8`
* 当前 Windows 控制台默认代码页仍可能是 `936`
* 仅重配 Python `stdout/stderr` 还不足以覆盖控制台代码页和子进程继承链路

## Requirements

* 补强 Trellis Python 公共入口的 UTF-8 初始化逻辑
* 提供一个仓库内可直接执行的 PowerShell UTF-8 初始化脚本
* 必要时补充文档说明，告诉用户和 AI 如何使用新的稳定入口

## Acceptance Criteria

* [ ] 高频 Trellis Python 入口在 Windows 下运行时会主动切换到 UTF-8 友好模式
* [ ] 仓库内存在一个可复用的 PowerShell UTF-8 初始化脚本
* [ ] 本次改动不影响任务系统和现有业务代码

## Out of Scope

* 修改业务功能代码
* 覆盖所有用户自定义终端配置
* 处理仓库外部工具的全部编码兼容性

## Technical Notes

* 优先改动 `.trellis/scripts/common/__init__.py`
* 可使用 Windows 控制台 API 将输入输出代码页切到 `65001`
* 可同步设置 `PYTHONIOENCODING` 与 `PYTHONUTF8`
