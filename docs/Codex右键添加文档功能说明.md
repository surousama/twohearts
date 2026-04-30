# Codex右键添加文档功能说明

这个功能通过一个本地 VS Code 小扩展实现。

## 功能效果

1.在左侧资源管理器中右键点击文件
2.选择“添加到 Codex 对话”
3.扩展会调用官方 Codex 扩展已有的 `Add File to Codex Thread` 命令

## 当前方案说明

1.不修改官方 Codex 扩展
2.只做一个桥接层，把官方已有能力挂到资源管理器右键菜单
3.这样兼容性和维护成本都更好

## 目录

扩展源码位于：`tools/codex-explorer-context`
